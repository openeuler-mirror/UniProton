/****************************************************************************
 * fs/ramfs/ramfs_vfsops.c
 *
 * Simple in-memory mountpoint used by UniProton RAMFS.
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/stat.h>
#include <sys/statfs.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <nuttx/fs/fs.h>
#include <nuttx/kmalloc.h>

#define RAMFS_SUPER_MAGIC  0x858458f6
#define RAMFS_DEFAULT_CAPACITY (256 * 1024)

struct ramfs_node {
    char name[NAME_MAX + 1];
    bool is_dir;
    unsigned int refs;
    size_t size;
    char *data;
    struct ramfs_node *parent;
    struct ramfs_node *child;
    struct ramfs_node *sibling;
};

struct ramfs_mountpt {
    struct ramfs_node root;
    mutex_t lock;
    size_t capacity;
    size_t used;
    unsigned long magic;
    struct inode *driver;
};

struct ramfs_dir_s {
    struct fs_dirent_s base;
    struct ramfs_node *node;
    struct ramfs_node *next;
};

static void ramfs_name_copy(char *dst, size_t dst_size, const char *src)
{
    if (dst_size == 0) {
        return;
    }

    (void)strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

static const char *ramfs_skip_slash(const char *path)
{
    while (path != NULL && *path == '/') {
        path++;
    }
    return path == NULL ? "" : path;
}

static struct ramfs_node *ramfs_child_find(struct ramfs_node *dir, const char *name, size_t len)
{
    struct ramfs_node *node;

    for (node = dir->child; node != NULL; node = node->sibling) {
        if (strlen(node->name) == len && strncmp(node->name, name, len) == 0) {
            return node;
        }
    }

    return NULL;
}

static struct ramfs_node *ramfs_find(struct ramfs_mountpt *fs, const char *path,
                                     struct ramfs_node **parent, const char **name)
{
    struct ramfs_node *walk = &fs->root;
    const char *cur = ramfs_skip_slash(path);

    if (parent != NULL) {
        *parent = NULL;
    }
    if (name != NULL) {
        *name = cur;
    }

    if (*cur == '\0') {
        return walk;
    }

    while (*cur != '\0') {
        const char *slash;
        size_t len;
        struct ramfs_node *child;

        if (!walk->is_dir) {
            return NULL;
        }

        slash = strchr(cur, '/');
        len = slash == NULL ? strlen(cur) : (size_t)(slash - cur);
        if (len == 0 || len > NAME_MAX) {
            return NULL;
        }

        child = ramfs_child_find(walk, cur, len);
        if (child == NULL) {
            if (parent != NULL) {
                *parent = walk;
            }
            if (name != NULL) {
                *name = cur;
            }
            return NULL;
        }

        walk = child;
        if (slash == NULL) {
            if (parent != NULL) {
                *parent = walk->parent;
            }
            if (name != NULL) {
                *name = cur;
            }
            return walk;
        }

        cur = ramfs_skip_slash(slash);
    }

    return walk;
}

static bool ramfs_name_is_leaf(const char *name)
{
    const char *slash;

    if (name == NULL || *name == '\0') {
        return false;
    }

    slash = strchr(name, '/');
    return slash == NULL || *ramfs_skip_slash(slash) == '\0';
}

static size_t ramfs_leaf_len(const char *name)
{
    const char *slash = strchr(name, '/');
    return slash == NULL ? strlen(name) : (size_t)(slash - name);
}

static struct ramfs_node *ramfs_create_node(struct ramfs_node *parent, const char *name, bool is_dir)
{
    struct ramfs_node *node;
    size_t len = ramfs_leaf_len(name);

    if (parent == NULL || !parent->is_dir || len == 0 || len > NAME_MAX || !ramfs_name_is_leaf(name)) {
        return NULL;
    }

    node = kmm_zalloc(sizeof(*node));
    if (node == NULL) {
        return NULL;
    }

    memcpy(node->name, name, len);
    node->name[len] = '\0';
    node->is_dir = is_dir;
    node->parent = parent;
    node->sibling = parent->child;
    parent->child = node;
    return node;
}

static void ramfs_remove_from_parent(struct ramfs_node *node)
{
    struct ramfs_node **link;

    if (node == NULL || node->parent == NULL) {
        return;
    }

    for (link = &node->parent->child; *link != NULL; link = &(*link)->sibling) {
        if (*link == node) {
            *link = node->sibling;
            return;
        }
    }
}

static void ramfs_free_tree(struct ramfs_mountpt *fs, struct ramfs_node *node)
{
    struct ramfs_node *child;

    while (node->child != NULL) {
        child = node->child;
        node->child = child->sibling;
        ramfs_free_tree(fs, child);
    }

    if (node->data != NULL) {
        fs->used -= node->size;
        kmm_free(node->data);
    }

    if (node != &fs->root) {
        kmm_free(node);
    }
}

static int ramfs_resize_file(struct ramfs_mountpt *fs, struct ramfs_node *node, size_t size)
{
    char *data;
    size_t oldsize = node->size;

    if (size > oldsize && fs->capacity != 0 && fs->used + (size - oldsize) > fs->capacity) {
        return -ENOSPC;
    }

    if (size == 0) {
        if (node->data != NULL) {
            kmm_free(node->data);
        }
        fs->used -= oldsize;
        node->data = NULL;
        node->size = 0;
        return OK;
    }

    data = kmm_realloc(node->data, size);
    if (data == NULL) {
        return -ENOMEM;
    }

    if (size > oldsize) {
        memset(data + oldsize, 0, size - oldsize);
        fs->used += size - oldsize;
    } else {
        fs->used -= oldsize - size;
    }

    node->data = data;
    node->size = size;
    return OK;
}

static int ramfs_open(struct file *filep, const char *relpath, int oflags, mode_t mode)
{
    struct ramfs_mountpt *fs = filep->f_inode->i_private;
    struct ramfs_node *node;
    struct ramfs_node *parent;
    const char *name;
    int ret;

    (void)mode;
    ret = nxmutex_lock(&fs->lock);
    if (ret < 0) {
        return ret;
    }

    node = ramfs_find(fs, relpath, &parent, &name);
    if (node == NULL) {
        if ((oflags & O_CREAT) == 0) {
            ret = -ENOENT;
            goto out;
        }

        node = ramfs_create_node(parent, name, false);
        if (node == NULL) {
            ret = -ENOENT;
            goto out;
        }
    } else {
        if (node->is_dir) {
            ret = -EISDIR;
            goto out;
        }

        if ((oflags & O_CREAT) != 0 && (oflags & O_EXCL) != 0) {
            ret = -EEXIST;
            goto out;
        }
    }

    if ((oflags & O_TRUNC) != 0 && (oflags & O_ACCMODE) != O_RDONLY) {
        ret = ramfs_resize_file(fs, node, 0);
        if (ret < 0) {
            goto out;
        }
    }

    node->refs++;
    filep->f_priv = node;
    filep->f_pos = (oflags & O_APPEND) != 0 ? (off_t)node->size : 0;
    ret = OK;

out:
    nxmutex_unlock(&fs->lock);
    return ret;
}

static int ramfs_close(struct file *filep)
{
    struct ramfs_mountpt *fs = filep->f_inode->i_private;
    struct ramfs_node *node = filep->f_priv;

    if (node == NULL) {
        return -EINVAL;
    }

    nxmutex_lock(&fs->lock);
    if (node->refs > 0) {
        node->refs--;
    }
    filep->f_priv = NULL;
    nxmutex_unlock(&fs->lock);
    return OK;
}

static ssize_t ramfs_read(struct file *filep, char *buffer, size_t buflen)
{
    struct ramfs_mountpt *fs = filep->f_inode->i_private;
    struct ramfs_node *node = filep->f_priv;
    size_t left;

    if (node == NULL || buffer == NULL) {
        return -EINVAL;
    }

    nxmutex_lock(&fs->lock);
    if (filep->f_pos < 0 || (size_t)filep->f_pos >= node->size) {
        nxmutex_unlock(&fs->lock);
        return 0;
    }

    left = node->size - (size_t)filep->f_pos;
    if (buflen > left) {
        buflen = left;
    }
    memcpy(buffer, node->data + filep->f_pos, buflen);
    filep->f_pos += buflen;
    nxmutex_unlock(&fs->lock);
    return (ssize_t)buflen;
}

static ssize_t ramfs_write(struct file *filep, const char *buffer, size_t buflen)
{
    struct ramfs_mountpt *fs = filep->f_inode->i_private;
    struct ramfs_node *node = filep->f_priv;
    size_t endpos;
    int ret;

    if (node == NULL || buffer == NULL) {
        return -EINVAL;
    }

    nxmutex_lock(&fs->lock);
    if (filep->f_pos < 0) {
        filep->f_pos = 0;
    }

    endpos = (size_t)filep->f_pos + buflen;
    if (endpos > node->size) {
        ret = ramfs_resize_file(fs, node, endpos);
        if (ret < 0) {
            nxmutex_unlock(&fs->lock);
            return ret;
        }
    }

    memcpy(node->data + filep->f_pos, buffer, buflen);
    filep->f_pos += buflen;
    nxmutex_unlock(&fs->lock);
    return (ssize_t)buflen;
}

static off_t ramfs_seek(struct file *filep, off_t offset, int whence)
{
    struct ramfs_mountpt *fs = filep->f_inode->i_private;
    struct ramfs_node *node = filep->f_priv;
    off_t pos;

    if (node == NULL) {
        return -EINVAL;
    }

    nxmutex_lock(&fs->lock);
    if (whence == SEEK_SET) {
        pos = offset;
    } else if (whence == SEEK_CUR) {
        pos = filep->f_pos + offset;
    } else if (whence == SEEK_END) {
        pos = (off_t)node->size + offset;
    } else {
        nxmutex_unlock(&fs->lock);
        return -EINVAL;
    }

    if (pos < 0) {
        pos = 0;
    }
    filep->f_pos = pos;
    nxmutex_unlock(&fs->lock);
    return pos;
}

static int ramfs_truncate(struct file *filep, off_t length)
{
    struct ramfs_mountpt *fs = filep->f_inode->i_private;
    struct ramfs_node *node = filep->f_priv;
    int ret;

    if (node == NULL || length < 0) {
        return -EINVAL;
    }

    nxmutex_lock(&fs->lock);
    ret = ramfs_resize_file(fs, node, (size_t)length);
    if (ret == OK && filep->f_pos > length) {
        filep->f_pos = length;
    }
    nxmutex_unlock(&fs->lock);
    return ret;
}

static int ramfs_stat_node(struct ramfs_node *node, struct stat *buf)
{
    memset(buf, 0, sizeof(*buf));
    buf->st_mode = node->is_dir ? (S_IFDIR | 0777) : (S_IFREG | 0666);
    buf->st_size = node->is_dir ? 0 : (off_t)node->size;
    buf->st_blksize = 512;
    buf->st_blocks = (buf->st_size + 511) / 512;
    return OK;
}

static int ramfs_fstat(const struct file *filep, struct stat *buf)
{
    struct ramfs_mountpt *fs = filep->f_inode->i_private;
    struct ramfs_node *node = filep->f_priv;
    int ret;

    if (node == NULL || buf == NULL) {
        return -EINVAL;
    }

    nxmutex_lock(&fs->lock);
    ret = ramfs_stat_node(node, buf);
    nxmutex_unlock(&fs->lock);
    return ret;
}

static int ramfs_opendir(struct inode *mountpt, const char *relpath, struct fs_dirent_s **dir)
{
    struct ramfs_mountpt *fs = mountpt->i_private;
    struct ramfs_node *node;
    struct ramfs_dir_s *mdir;
    int ret;

    ret = nxmutex_lock(&fs->lock);
    if (ret < 0) {
        return ret;
    }

    node = ramfs_find(fs, relpath, NULL, NULL);
    if (node == NULL) {
        ret = -ENOENT;
        goto out;
    }
    if (!node->is_dir) {
        ret = -ENOTDIR;
        goto out;
    }

    mdir = kmm_zalloc(sizeof(*mdir));
    if (mdir == NULL) {
        ret = -ENOMEM;
        goto out;
    }

    node->refs++;
    mdir->node = node;
    mdir->next = node->child;
    *dir = &mdir->base;
    ret = OK;

out:
    nxmutex_unlock(&fs->lock);
    return ret;
}

static int ramfs_closedir(struct inode *mountpt, struct fs_dirent_s *dir)
{
    struct ramfs_mountpt *fs = mountpt->i_private;
    struct ramfs_dir_s *mdir = (struct ramfs_dir_s *)dir;

    nxmutex_lock(&fs->lock);
    if (mdir->node->refs > 0) {
        mdir->node->refs--;
    }
    nxmutex_unlock(&fs->lock);
    kmm_free(mdir);
    return OK;
}

static int ramfs_readdir(struct inode *mountpt, struct fs_dirent_s *dir, struct dirent *entry)
{
    struct ramfs_mountpt *fs = mountpt->i_private;
    struct ramfs_dir_s *mdir = (struct ramfs_dir_s *)dir;
    struct ramfs_node *node;

    nxmutex_lock(&fs->lock);
    node = mdir->next;
    if (node == NULL) {
        nxmutex_unlock(&fs->lock);
        return -ENOENT;
    }

    ramfs_name_copy(entry->d_name, sizeof(entry->d_name), node->name);
    entry->d_type = node->is_dir ? DTYPE_DIRECTORY : DTYPE_FILE;
    mdir->next = node->sibling;
    nxmutex_unlock(&fs->lock);
    return OK;
}

static int ramfs_rewinddir(struct inode *mountpt, struct fs_dirent_s *dir)
{
    struct ramfs_mountpt *fs = mountpt->i_private;
    struct ramfs_dir_s *mdir = (struct ramfs_dir_s *)dir;

    nxmutex_lock(&fs->lock);
    mdir->next = mdir->node->child;
    nxmutex_unlock(&fs->lock);
    return OK;
}

static int ramfs_bind_common(struct inode *driver, const void *data, void **handle, unsigned long magic)
{
    struct ramfs_mountpt *fs;
    uintptr_t capacity = (uintptr_t)data;

    fs = kmm_zalloc(sizeof(*fs));
    if (fs == NULL) {
        return -ENOMEM;
    }

    fs->root.is_dir = true;
    fs->capacity = capacity == 0 ? RAMFS_DEFAULT_CAPACITY : (size_t)capacity;
    fs->magic = magic;
    fs->driver = driver;
    nxmutex_init(&fs->lock);
    *handle = fs;
    return OK;
}

static int ramfs_unbind(void *handle, struct inode **driver, unsigned int flags)
{
    struct ramfs_mountpt *fs = handle;

    (void)flags;
    if (fs == NULL) {
        return -EINVAL;
    }

    nxmutex_lock(&fs->lock);
    ramfs_free_tree(fs, &fs->root);
    nxmutex_unlock(&fs->lock);
    nxmutex_destroy(&fs->lock);
    if (driver != NULL) {
        *driver = fs->driver;
    }
    kmm_free(fs);
    return OK;
}

static int ramfs_statfs(struct inode *mountpt, struct statfs *buf)
{
    struct ramfs_mountpt *fs = mountpt->i_private;

    memset(buf, 0, sizeof(*buf));
    nxmutex_lock(&fs->lock);
    buf->f_type = fs->magic;
    buf->f_bsize = 512;
    buf->f_blocks = fs->capacity / 512;
    buf->f_bfree = (fs->capacity - fs->used) / 512;
    buf->f_bavail = buf->f_bfree;
    buf->f_namelen = NAME_MAX;
    nxmutex_unlock(&fs->lock);
    return OK;
}

static int ramfs_unlink(struct inode *mountpt, const char *relpath)
{
    struct ramfs_mountpt *fs = mountpt->i_private;
    struct ramfs_node *node;
    int ret = OK;

    nxmutex_lock(&fs->lock);
    node = ramfs_find(fs, relpath, NULL, NULL);
    if (node == NULL) {
        ret = -ENOENT;
    } else if (node->is_dir) {
        ret = -EISDIR;
    } else if (node->refs != 0) {
        ret = -EBUSY;
    } else {
        ramfs_remove_from_parent(node);
        ramfs_free_tree(fs, node);
    }
    nxmutex_unlock(&fs->lock);
    return ret;
}

static int ramfs_mkdir(struct inode *mountpt, const char *relpath, mode_t mode)
{
    struct ramfs_mountpt *fs = mountpt->i_private;
    struct ramfs_node *node;
    struct ramfs_node *parent;
    const char *name;
    int ret = OK;

    (void)mode;
    nxmutex_lock(&fs->lock);
    node = ramfs_find(fs, relpath, &parent, &name);
    if (node != NULL) {
        ret = -EEXIST;
    } else if (ramfs_create_node(parent, name, true) == NULL) {
        ret = -ENOENT;
    }
    nxmutex_unlock(&fs->lock);
    return ret;
}

static int ramfs_rmdir(struct inode *mountpt, const char *relpath)
{
    struct ramfs_mountpt *fs = mountpt->i_private;
    struct ramfs_node *node;
    int ret = OK;

    nxmutex_lock(&fs->lock);
    node = ramfs_find(fs, relpath, NULL, NULL);
    if (node == NULL) {
        ret = -ENOENT;
    } else if (!node->is_dir) {
        ret = -ENOTDIR;
    } else if (node == &fs->root || node->child != NULL || node->refs != 0) {
        ret = -EBUSY;
    } else {
        ramfs_remove_from_parent(node);
        ramfs_free_tree(fs, node);
    }
    nxmutex_unlock(&fs->lock);
    return ret;
}

static int ramfs_rename(struct inode *mountpt, const char *oldrelpath, const char *newrelpath)
{
    struct ramfs_mountpt *fs = mountpt->i_private;
    struct ramfs_node *node;
    struct ramfs_node *parent;
    const char *name;
    size_t len;
    int ret = OK;

    nxmutex_lock(&fs->lock);
    node = ramfs_find(fs, oldrelpath, NULL, NULL);
    if (node == NULL) {
        ret = -ENOENT;
        goto out;
    }
    if (ramfs_find(fs, newrelpath, &parent, &name) != NULL) {
        ret = -EEXIST;
        goto out;
    }
    if (parent == NULL || parent != node->parent || !ramfs_name_is_leaf(name)) {
        ret = -EINVAL;
        goto out;
    }
    len = ramfs_leaf_len(name);
    if (len == 0 || len > NAME_MAX) {
        ret = -ENAMETOOLONG;
        goto out;
    }
    memcpy(node->name, name, len);
    node->name[len] = '\0';

out:
    nxmutex_unlock(&fs->lock);
    return ret;
}

static int ramfs_stat(struct inode *mountpt, const char *relpath, struct stat *buf)
{
    struct ramfs_mountpt *fs = mountpt->i_private;
    struct ramfs_node *node;
    int ret;

    nxmutex_lock(&fs->lock);
    node = ramfs_find(fs, relpath, NULL, NULL);
    if (node == NULL) {
        ret = -ENOENT;
    } else {
        ret = ramfs_stat_node(node, buf);
    }
    nxmutex_unlock(&fs->lock);
    return ret;
}

#ifdef CONFIG_FS_RAMFS
static int ramfs_bind(struct inode *driver, const void *data, void **handle)
{
    (void)driver;
    return ramfs_bind_common(NULL, data, handle, RAMFS_SUPER_MAGIC);
}

const struct mountpt_operations g_ramfs_operations = {
    ramfs_open, ramfs_close, ramfs_read, ramfs_write, ramfs_seek, NULL, NULL,
    ramfs_truncate, NULL, NULL, ramfs_fstat, NULL, ramfs_opendir,
    ramfs_closedir, ramfs_readdir, ramfs_rewinddir, ramfs_bind,
    ramfs_unbind, ramfs_statfs, ramfs_unlink, ramfs_mkdir, ramfs_rmdir,
    ramfs_rename, ramfs_stat, NULL
};
#endif
