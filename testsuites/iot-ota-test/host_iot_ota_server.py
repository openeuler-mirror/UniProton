#!/usr/bin/env python3
"""Host-side NB-IoT proxy, MQTT broker and LwM2M/CoAP server for sd3403 OTA tests."""

import argparse
import atexit
import json
import os
import queue
import shutil
import signal
import socket
import struct
import subprocess
import sys
import tempfile
import threading
import time
import urllib.error
import urllib.parse
import urllib.request
import zlib


PREFIX = b"IOTOTA1"
FIRMWARE = (
    b"UniProton sd3403 OTA firmware image via LwM2M manifest, MQTT chunks and NB-IoT proxy report."
    b" Version two payload for end to end upgrade validation.\0"
)
VERSION = "2.0.0"
MQTT_TOPIC_REQUEST = "ota/request"
MQTT_TOPIC_CHUNK = "ota/chunk"
MQTT_TOPIC_MANIFEST_REQUEST = "ota/manifest/request"
MQTT_TOPIC_MANIFEST = "ota/manifest"
MQTT_DEMO_TOPIC_REPORT = "demo/report"
MQTT_DEMO_TOPIC_COMMAND = "demo/command"
MQTT_DEMO_TOPIC_RESPONSE = "demo/response"
MQTT_DEMO_COMMAND = (
    b'{"msgType":"cloudReq","serviceId":"Battery","cmd":"SetReportPeriod",'
    b'"paras":{"period":10},"hasMore":0,"mid":1001}'
)
MQTT_DEMO_COMMAND_MID = 1001
CHILD_PROCS = []
TEMP_FILES = []
THREAD_FAILURES = queue.Queue()
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "..", ".."))
DEFAULT_LESHAN_SERVER_JAR = os.path.join(REPO_ROOT, "tools", "leshan-server-demo.jar")
LESHAN_SERVER_JAR = os.environ.get("LESHAN_SERVER_JAR", DEFAULT_LESHAN_SERVER_JAR)
LWM2M_ENDPOINT = "uniproton-sd3403"


def be16(value):
    return bytes(((value >> 8) & 0xFF, value & 0xFF))


def be32(value):
    return bytes(((value >> 24) & 0xFF, (value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF))


def build_package():
    head_len = 20
    total_len = head_len + len(FIRMWARE)
    header = bytearray()
    header += be32(0)          # OTA package version
    header += be32(head_len)
    header += be32(total_len)
    header += be16(4)          # PACK_TLV_T_BIN_TYPE
    header += be16(4)
    header += be32(0)          # OTA_FULL_SOFTWARE
    return bytes(header) + FIRMWARE


PACKAGE = build_package()


def thread_entry(name, target, args):
    try:
        target(*args)
    except Exception as exc:
        print(f"[HOST_IOT_OTA_SERVER] FATAL thread {name} failed: {exc}", flush=True)
        THREAD_FAILURES.put((name, exc))


def make_thread(name, target, args=(), daemon=True):
    return threading.Thread(target=thread_entry, args=(name, target, args), daemon=daemon, name=name)


def mqtt_chunk_payload(offset, size):
    if offset < 0 or size <= 0 or offset + size > len(PACKAGE):
        raise ValueError(f"bad MQTT OTA chunk request offset={offset} size={size}")
    chunk = PACKAGE[offset:offset + size]
    crc = zlib.crc32(chunk) & 0xFFFFFFFF
    header = f"CHUNK {offset} {size} {crc:08x}\n".encode("ascii")
    return header + chunk


class NbState:
    def __init__(self):
        self.nb_iot_connected = False
        self.last_report = ""
        self.report_count = 0


class CoapState:
    def __init__(self, sock):
        self.sock = sock
        self.lock = threading.Lock()
        self.pending = {}
        self.demo_started = set()
        self.ota_started = set()


def read_full(conn, size):
    data = bytearray()
    while len(data) < size:
        chunk = conn.recv(size - len(data))
        if not chunk:
            raise ConnectionError("connection closed")
        data.extend(chunk)
    return bytes(data)


def recv_frame(conn):
    line = bytearray()
    while True:
        ch = conn.recv(1)
        if not ch:
            raise ConnectionError("connection closed while reading frame header")
        if ch == b"\n":
            break
        line.extend(ch)

    parts = bytes(line).split()
    if len(parts) != 3 or parts[0] != PREFIX:
        raise ValueError(f"bad frame header: {bytes(line)!r}")
    channel_len = int(parts[1])
    data_len = int(parts[2])
    channel = read_full(conn, channel_len).decode("ascii")
    data = read_full(conn, data_len)
    return channel, data


def send_frame(conn, channel, data):
    channel_b = channel.encode("ascii")
    header = PREFIX + b" " + str(len(channel_b)).encode() + b" " + str(len(data)).encode() + b"\n"
    conn.sendall(header + channel_b + data)


def handle_nb_iot_request(state, channel, data):
    text = data.decode("utf-8", errors="replace")
    print(f"[HOST_IOT_OTA_SERVER] NB-IoT rx channel={channel} data={text!r}", flush=True)

    if channel == "nb_iot":
        if text.startswith("CONNECT "):
            state.nb_iot_connected = True
            return "nb_iot", b"OK"
        if text.startswith("REPORT ") and state.nb_iot_connected:
            state.last_report = text[7:]
            state.report_count += 1
            return "nb_iot", b"OK"

    return channel, b"ERR"


def serve_nb_iot_client(conn, addr):
    print(f"[HOST_IOT_OTA_SERVER] NB-IoT client connected addr={addr}", flush=True)
    state = NbState()
    try:
        while True:
            channel, data = recv_frame(conn)
            resp_channel, resp_data = handle_nb_iot_request(state, channel, data)
            send_frame(conn, resp_channel, resp_data)
    except ConnectionError:
        print(
            "[HOST_IOT_OTA_SERVER] NB-IoT client closed "
            f"nb_iot={state.nb_iot_connected} reports={state.report_count} last_report={state.last_report!r}",
            flush=True,
        )
    finally:
        conn.close()


def nb_iot_proxy_server(host, port):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as srv:
        srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        srv.bind((host, port))
        srv.listen(8)
        print(f"[HOST_IOT_OTA_SERVER] NB-IoT proxy listening on {host}:{port}", flush=True)
        while True:
            conn, addr = srv.accept()
            thread = make_thread("nb-iot-client", serve_nb_iot_client, (conn, addr))
            thread.start()


def mqtt_encode_remaining(length):
    out = bytearray()
    while True:
        digit = length % 128
        length //= 128
        if length:
            digit |= 0x80
        out.append(digit)
        if not length:
            break
    return bytes(out)


def mqtt_read_packet(conn):
    first = read_full(conn, 1)[0]
    multiplier = 1
    remaining = 0
    while True:
        digit = read_full(conn, 1)[0]
        remaining += (digit & 127) * multiplier
        if not digit & 128:
            break
        multiplier *= 128
    payload = read_full(conn, remaining) if remaining else b""
    return first >> 4, payload


def mqtt_send_packet(conn, packet_type, flags, payload=b""):
    conn.sendall(bytes(((packet_type << 4) | flags,)) + mqtt_encode_remaining(len(payload)) + payload)


def mqtt_read_string(payload, pos):
    if pos + 2 > len(payload):
        raise ValueError("bad mqtt string")
    length = (payload[pos] << 8) | payload[pos + 1]
    pos += 2
    if pos + length > len(payload):
        raise ValueError("bad mqtt string length")
    return payload[pos:pos + length].decode("utf-8"), pos + length


def mqtt_publish(conn, topic, payload):
    topic_b = topic.encode("utf-8")
    mqtt_send_packet(conn, 3, 0, struct.pack("!H", len(topic_b)) + topic_b + payload)


def validate_mqtt_demo_report(body):
    try:
        report = json.loads(body)
        data = report.get("data")
        if report.get("msgType") != "deviceReq" or report.get("hasMore") != 0:
            return False
        if not isinstance(data, list) or len(data) != 1:
            return False
        service = data[0]
        service_data = service.get("serviceData") if isinstance(service, dict) else None
        return (
            service.get("serviceId") == "Battery" and
            isinstance(service_data, dict) and
            isinstance(service_data.get("batteryLevel"), int)
        )
    except (TypeError, ValueError, json.JSONDecodeError):
        return False


def validate_mqtt_demo_response(body):
    try:
        response = json.loads(body)
        body_obj = response.get("body")
        return (
            response.get("msgType") == "deviceRsp" and
            response.get("mid") == MQTT_DEMO_COMMAND_MID and
            response.get("errcode") == 0 and
            response.get("hasMore") == 0 and
            isinstance(body_obj, dict) and
            body_obj.get("result") == "ok"
        )
    except (TypeError, ValueError, json.JSONDecodeError):
        return False


def serve_builtin_mqtt_client(conn, addr, enable_demo, enable_ota):
    print(f"[HOST_IOT_OTA_SERVER] MQTT client connected addr={addr}", flush=True)
    try:
        packet_type, payload = mqtt_read_packet(conn)
        if packet_type != 1:
            return
        client_id = ""
        try:
            pos = 0
            _proto, pos = mqtt_read_string(payload, pos)
            pos += 4
            client_id, pos = mqtt_read_string(payload, pos)
        except ValueError:
            pass
        print(f"[HOST_IOT_OTA_SERVER] MQTT CONNECT client_id={client_id!r}", flush=True)
        mqtt_send_packet(conn, 2, 0, b"\x00\x00")

        while True:
            packet_type, payload = mqtt_read_packet(conn)
            if packet_type == 8:
                packet_id = payload[:2]
                topic, _ = mqtt_read_string(payload, 2)
                print(f"[HOST_IOT_OTA_SERVER] MQTT SUBSCRIBE topic={topic!r}", flush=True)
                mqtt_send_packet(conn, 9, 0, packet_id + b"\x00")
            elif packet_type == 3:
                topic, pos = mqtt_read_string(payload, 0)
                body = payload[pos:].decode("ascii", errors="replace")
                print(f"[HOST_IOT_OTA_SERVER] MQTT PUBLISH topic={topic!r} body={body!r}", flush=True)
                if enable_ota and topic == MQTT_TOPIC_REQUEST and body.startswith("GET_CHUNK "):
                    _, offset_s, size_s = body.split(maxsplit=2)
                    offset = int(offset_s)
                    size = int(size_s)
                    mqtt_publish(conn, MQTT_TOPIC_CHUNK, mqtt_chunk_payload(offset, size))
                elif enable_ota and topic == MQTT_TOPIC_MANIFEST_REQUEST and body == "GET_MANIFEST":
                    crc = zlib.crc32(PACKAGE) & 0xFFFFFFFF
                    manifest = f"MANIFEST {VERSION} {len(PACKAGE)} {crc:08x}".encode("ascii")
                    mqtt_publish(conn, MQTT_TOPIC_MANIFEST, manifest)
                elif enable_demo and topic == MQTT_DEMO_TOPIC_REPORT:
                    if validate_mqtt_demo_report(body):
                        print("[HOST_IOT_OTA_SERVER] MQTT demo JSON report valid; sending cloud command", flush=True)
                        mqtt_publish(conn, MQTT_DEMO_TOPIC_COMMAND, MQTT_DEMO_COMMAND)
                    else:
                        print(f"[HOST_IOT_OTA_SERVER] invalid MQTT demo JSON report={body!r}", flush=True)
                elif enable_demo and topic == MQTT_DEMO_TOPIC_RESPONSE:
                    if validate_mqtt_demo_response(body):
                        print("[HOST_IOT_OTA_SERVER] MQTT demo JSON response valid", flush=True)
                    else:
                        print(f"[HOST_IOT_OTA_SERVER] invalid MQTT demo JSON response={body!r}", flush=True)
            elif packet_type == 14:
                return
    except ConnectionError:
        print("[HOST_IOT_OTA_SERVER] MQTT client closed", flush=True)
    finally:
        conn.close()


def builtin_mqtt_server(host, port, enable_demo, enable_ota):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as srv:
        srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        srv.bind((host, port))
        srv.listen(8)
        print(f"[HOST_IOT_OTA_SERVER] builtin MQTT broker listening on {host}:{port}", flush=True)
        while True:
            conn, addr = srv.accept()
            thread = make_thread("builtin-mqtt-client", serve_builtin_mqtt_client,
                (conn, addr, enable_demo, enable_ota))
            thread.start()


def mqtt_put_string(buf, value):
    data = value.encode("utf-8")
    buf.extend(struct.pack("!H", len(data)))
    buf.extend(data)


def mqtt_client_connect(conn, client_id):
    payload = bytearray()
    mqtt_put_string(payload, "MQTT")
    payload.extend(b"\x04\x02\x00\x3c")
    mqtt_put_string(payload, client_id)
    mqtt_send_packet(conn, 1, 0, bytes(payload))
    packet_type, resp = mqtt_read_packet(conn)
    return packet_type == 2 and resp == b"\x00\x00"


def mqtt_client_subscribe(conn, topic):
    payload = bytearray(b"\x00\x01")
    mqtt_put_string(payload, topic)
    payload.append(0)
    mqtt_send_packet(conn, 8, 2, bytes(payload))
    packet_type, resp = mqtt_read_packet(conn)
    return packet_type == 9 and len(resp) >= 3


def mqtt_test_helper(host, port, enable_demo, enable_ota):
    while True:
        try:
            with socket.create_connection((host, port), timeout=5) as conn:
                conn.settimeout(None)
                if not mqtt_client_connect(conn, "sd3403-host-helper"):
                    raise ConnectionError("helper MQTT CONNACK failed")
                if enable_ota and not mqtt_client_subscribe(conn, MQTT_TOPIC_REQUEST):
                    raise ConnectionError("helper MQTT SUBACK failed")
                if enable_ota and not mqtt_client_subscribe(conn, MQTT_TOPIC_MANIFEST_REQUEST):
                    raise ConnectionError("helper MQTT manifest SUBACK failed")
                if enable_demo and not mqtt_client_subscribe(conn, MQTT_DEMO_TOPIC_REPORT):
                    raise ConnectionError("helper MQTT demo report SUBACK failed")
                if enable_demo and not mqtt_client_subscribe(conn, MQTT_DEMO_TOPIC_RESPONSE):
                    raise ConnectionError("helper MQTT demo response SUBACK failed")
                print(
                    "[HOST_IOT_OTA_SERVER] Mosquitto helper subscribed "
                    f"demo={enable_demo} ota={enable_ota}",
                    flush=True,
                )
                while True:
                    packet_type, payload = mqtt_read_packet(conn)
                    if packet_type != 3:
                        continue
                    topic, pos = mqtt_read_string(payload, 0)
                    body = payload[pos:].decode("ascii", errors="replace")
                    print(f"[HOST_IOT_OTA_SERVER] Mosquitto helper rx topic={topic!r} body={body!r}", flush=True)
                    if enable_ota and topic == MQTT_TOPIC_REQUEST and body.startswith("GET_CHUNK "):
                        _, offset_s, size_s = body.split(maxsplit=2)
                        offset = int(offset_s)
                        size = int(size_s)
                        mqtt_publish(conn, MQTT_TOPIC_CHUNK, mqtt_chunk_payload(offset, size))
                    elif enable_ota and topic == MQTT_TOPIC_MANIFEST_REQUEST and body == "GET_MANIFEST":
                        crc = zlib.crc32(PACKAGE) & 0xFFFFFFFF
                        manifest = f"MANIFEST {VERSION} {len(PACKAGE)} {crc:08x}".encode("ascii")
                        mqtt_publish(conn, MQTT_TOPIC_MANIFEST, manifest)
                    elif enable_demo and topic == MQTT_DEMO_TOPIC_REPORT:
                        if validate_mqtt_demo_report(body):
                            print("[HOST_IOT_OTA_SERVER] Mosquitto helper got valid MQTT demo JSON report", flush=True)
                            mqtt_publish(conn, MQTT_DEMO_TOPIC_COMMAND, MQTT_DEMO_COMMAND)
                        else:
                            print(f"[HOST_IOT_OTA_SERVER] invalid Mosquitto MQTT demo JSON report={body!r}", flush=True)
                    elif enable_demo and topic == MQTT_DEMO_TOPIC_RESPONSE:
                        if validate_mqtt_demo_response(body):
                            print("[HOST_IOT_OTA_SERVER] Mosquitto helper got valid MQTT demo JSON response", flush=True)
                        else:
                            print(f"[HOST_IOT_OTA_SERVER] invalid Mosquitto MQTT demo JSON response={body!r}", flush=True)
        except (ConnectionError, OSError) as exc:
            print(f"[HOST_IOT_OTA_SERVER] Mosquitto helper reconnecting: {exc}", flush=True)
            time.sleep(1)


def copy_proc_output(proc, prefix):
    assert proc.stdout is not None
    for line in proc.stdout:
        print(f"[HOST_IOT_OTA_SERVER] {prefix} {line.rstrip()}", flush=True)


def cleanup_children():
    for proc in CHILD_PROCS:
        if proc.poll() is None:
            proc.terminate()
            try:
                proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                proc.kill()
    for path in TEMP_FILES:
        try:
            os.unlink(path)
        except OSError:
            pass


atexit.register(cleanup_children)


def handle_stop(signum, frame):
    del signum, frame
    cleanup_children()
    sys.exit(0)


signal.signal(signal.SIGINT, handle_stop)
signal.signal(signal.SIGTERM, handle_stop)


def start_mosquitto_backend(host, port, enable_demo, enable_ota):
    del host
    mosquitto = shutil.which("mosquitto")
    if mosquitto is None:
        return False

    try:
        with socket.create_connection(("127.0.0.1", port), timeout=1):
            print(f"[HOST_IOT_OTA_SERVER] reusing existing Mosquitto broker on 127.0.0.1:{port}", flush=True)
            return [make_thread("mosquitto-helper", mqtt_test_helper,
                ("127.0.0.1", port, enable_demo, enable_ota))]
    except OSError:
        pass

    with tempfile.NamedTemporaryFile("w", delete=False, prefix="iot-ota-mosquitto-", suffix=".conf") as conf:
        conf.write(f"listener {port} 0.0.0.0\n")
        conf.write("allow_anonymous true\n")
        conf.write("persistence false\n")
        conf.write("log_type all\n")
        conf_path = conf.name
    TEMP_FILES.append(conf_path)

    proc = subprocess.Popen(
        [mosquitto, "-c", conf_path, "-v"],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    CHILD_PROCS.append(proc)
    make_thread("mosquitto-log", copy_proc_output, (proc, "mosquitto:")).start()
    time.sleep(1)
    if proc.poll() is not None:
        return False
    print(f"[HOST_IOT_OTA_SERVER] Mosquitto broker listening on 0.0.0.0:{port}", flush=True)
    return [make_thread("mosquitto-helper", mqtt_test_helper,
        ("127.0.0.1", port, enable_demo, enable_ota))]


def start_mosquitto_container_backend(host, port, enable_demo, enable_ota):
    del host
    docker = shutil.which("docker")
    if docker is None:
        return False

    name = f"iot-ota-mosquitto-{port}"
    subprocess.run([docker, "rm", "-f", name], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    proc = subprocess.Popen(
        [
            docker,
            "run",
            "--rm",
            "--name",
            name,
            "-p",
            f"{port}:1883",
            "eclipse-mosquitto:2",
            "mosquitto",
            "-c",
            "/mosquitto-no-auth.conf",
            "-v",
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    CHILD_PROCS.append(proc)
    make_thread("mosquitto-container-log", copy_proc_output, (proc, "mosquitto-container:")).start()
    time.sleep(3)
    if proc.poll() is not None:
        return False
    print(f"[HOST_IOT_OTA_SERVER] Mosquitto container broker listening on 0.0.0.0:{port}", flush=True)
    return [make_thread("mosquitto-container-helper", mqtt_test_helper,
        ("127.0.0.1", port, enable_demo, enable_ota))]


def start_mqtt_backend(host, port, backend, enable_demo, enable_ota):
    if backend in ("mosquitto", "auto"):
        threads = start_mosquitto_backend(host, port, enable_demo, enable_ota)
        if threads:
            return threads
    if backend in ("mosquitto", "auto"):
        threads = start_mosquitto_container_backend(host, port, enable_demo, enable_ota)
        if threads:
            return threads
    if backend == "mosquitto":
        raise RuntimeError("mosquitto is required but was not found or failed to start")
    if backend == "auto":
        print(
            "[HOST_IOT_OTA_SERVER] WARNING: mosquitto unavailable; using builtin MQTT fallback. "
            "Prefer Mosquitto for protocol-level MQTT validation.",
            flush=True,
        )
    return [make_thread("builtin-mqtt-server", builtin_mqtt_server,
        (host, port, enable_demo, enable_ota))]


def start_leshan_backend(host, coap_port, web_port, leshan_jar):
    java = shutil.which("java")
    if java is None:
        raise RuntimeError("java is required for Leshan backend")
    if not os.path.exists(leshan_jar):
        raise RuntimeError(
            f"Leshan demo server jar not found: {leshan_jar}. "
            "Download leshan-server-demo-*-jar-with-dependencies.jar and pass --leshan-jar, "
            "or place it at tools/leshan-server-demo.jar."
        )

    proc = subprocess.Popen(
        [
            java,
            "-jar",
            leshan_jar,
            "--coap-host",
            host,
            "--coap-port",
            str(coap_port),
            "--web-host",
            host,
            "--web-port",
            str(web_port),
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    CHILD_PROCS.append(proc)
    make_thread("leshan-log", copy_proc_output, (proc, "leshan:")).start()
    time.sleep(5)
    if proc.poll() is not None:
        raise RuntimeError("Leshan server exited early")
    print(
        f"[HOST_IOT_OTA_SERVER] Leshan LwM2M server listening on {host}:{coap_port}, web {host}:{web_port}",
        flush=True,
    )


def http_request(method, url, body=None, timeout=5):
    data = None
    headers = {}
    if body is not None:
        data = body.encode("utf-8")
        headers["Content-Type"] = "application/json"
    req = urllib.request.Request(url, data=data, headers=headers, method=method)
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        return resp.status, resp.read().decode("utf-8", errors="replace")


def wait_for_leshan_client(web_host, web_port, endpoint, timeout=60):
    deadline = time.time() + timeout
    url = f"http://{web_host}:{web_port}/api/clients"
    while time.time() < deadline:
        try:
            status, text = http_request("GET", url, timeout=3)
            if status == 200:
                clients = json.loads(text)
                for client in clients:
                    if client.get("endpoint") == endpoint:
                        return client
        except (OSError, urllib.error.URLError, json.JSONDecodeError):
            pass
        time.sleep(1)
    return None


def leshan_write_resource(web_host, web_port, endpoint, path, value, value_type="STRING"):
    quoted_endpoint = urllib.parse.quote(endpoint, safe="")
    url = f"http://{web_host}:{web_port}/api/clients/{quoted_endpoint}{path}?format=TEXT"
    body = json.dumps({"kind": "singleResource", "id": int(path.rsplit("/", 1)[1]), "type": value_type, "value": value})
    status, text = http_request("PUT", url, body=body, timeout=10)
    if status < 200 or status >= 300:
        raise RuntimeError(f"Leshan write {path} failed status={status} body={text}")
    return text


def leshan_execute_resource(web_host, web_port, endpoint, path):
    quoted_endpoint = urllib.parse.quote(endpoint, safe="")
    url = f"http://{web_host}:{web_port}/api/clients/{quoted_endpoint}{path}"
    status, text = http_request("POST", url, timeout=10)
    if status < 200 or status >= 300:
        raise RuntimeError(f"Leshan execute {path} failed status={status} body={text}")
    return text


def leshan_read_resource(web_host, web_port, endpoint, path):
    quoted_endpoint = urllib.parse.quote(endpoint, safe="")
    url = f"http://{web_host}:{web_port}/api/clients/{quoted_endpoint}{path}"
    status, text = http_request("GET", url, timeout=10)
    if status < 200 or status >= 300:
        raise RuntimeError(f"Leshan read {path} failed status={status} body={text}")
    return text


def client_object_links(client):
    links = client.get("objectLinks") or client.get("objectLink") or client.get("links") or []
    if isinstance(links, str):
        return links
    return " ".join(str(link) for link in links)


def leshan_firmware_update_driver(web_host, web_port, endpoint):
    processed = set()
    while True:
        client = wait_for_leshan_client(web_host, web_port, endpoint)
        if client is None:
            print(f"[HOST_IOT_OTA_SERVER] Leshan endpoint {endpoint!r} not registered yet", flush=True)
            continue
        registration_id = client.get("registrationId") or endpoint
        links = client_object_links(client)
        if "/5/0" not in links:
            time.sleep(2)
            continue
        if registration_id in processed:
            time.sleep(2)
            continue
        try:
            print("[HOST_IOT_OTA_SERVER] Leshan WRITE /5/0/0 package bytes", flush=True)
            leshan_write_resource(web_host, web_port, endpoint, "/5/0/0", PACKAGE.hex(), "OPAQUE")
            print("[HOST_IOT_OTA_SERVER] Leshan EXECUTE /5/0/2", flush=True)
            leshan_execute_resource(web_host, web_port, endpoint, "/5/0/2")
            processed.add(registration_id)
        except (OSError, urllib.error.URLError, RuntimeError) as exc:
            print(f"[HOST_IOT_OTA_SERVER] Leshan firmware update driver retrying: {exc}", flush=True)
            time.sleep(2)


def leshan_demo_device_driver(web_host, web_port, endpoint):
    processed = set()
    while True:
        client = wait_for_leshan_client(web_host, web_port, endpoint)
        if client is None:
            print(f"[HOST_IOT_OTA_SERVER] Leshan endpoint {endpoint!r} not registered yet", flush=True)
            continue
        registration_id = client.get("registrationId") or endpoint
        links = client_object_links(client)
        if "/3/0" not in links or "/5/0" in links:
            time.sleep(2)
            continue
        if registration_id in processed:
            time.sleep(2)
            continue
        try:
            print("[HOST_IOT_OTA_SERVER] Leshan demo READ /3/0/0", flush=True)
            text = leshan_read_resource(web_host, web_port, endpoint, "/3/0/0")
            if "Open Mobile Alliance" not in text:
                raise RuntimeError(f"unexpected manufacturer read body={text}")
            print("[HOST_IOT_OTA_SERVER] Leshan demo WRITE /3/0/14", flush=True)
            leshan_write_resource(web_host, web_port, endpoint, "/3/0/14", "+08:00")
            print("[HOST_IOT_OTA_SERVER] Leshan demo EXECUTE /3/0/4", flush=True)
            leshan_execute_resource(web_host, web_port, endpoint, "/3/0/4")
            processed.add(registration_id)
        except (OSError, urllib.error.URLError, RuntimeError) as exc:
            print(f"[HOST_IOT_OTA_SERVER] Leshan demo device driver retrying: {exc}", flush=True)
            time.sleep(2)


def start_lwm2m_backend(host, lwm2m_port, manifest_port, web_port, backend, leshan_jar, enable_demo, enable_ota):
    del manifest_port
    if backend == "leshan":
        start_leshan_backend(host, lwm2m_port, web_port, leshan_jar)
        web_host = "127.0.0.1" if host == "0.0.0.0" else host
        threads = []
        if enable_ota:
            threads.append(make_thread("leshan-firmware-driver", leshan_firmware_update_driver,
                (web_host, web_port, LWM2M_ENDPOINT)))
        if enable_demo:
            threads.append(make_thread("leshan-demo-driver", leshan_demo_device_driver,
                (web_host, web_port, LWM2M_ENDPOINT)))
        return threads
    return [make_thread("builtin-lwm2m-server", coap_server, (host, lwm2m_port, enable_demo, enable_ota))]


def coap_get_payload(data):
    if b"\xff" not in data:
        return b""
    return data.split(b"\xff", 1)[1]


def coap_mid(data):
    return (data[2] << 8) | data[3]


def coap_parse_options(data):
    token_len = data[0] & 0x0F
    pos = 4 + token_len
    number = 0
    options = []
    while pos < len(data) and data[pos] != 0xFF:
        delta = data[pos] >> 4
        length = data[pos] & 0x0F
        pos += 1
        if delta == 13:
            delta = data[pos] + 13
            pos += 1
        elif delta == 14:
            delta = ((data[pos] << 8) | data[pos + 1]) + 269
            pos += 2
        if length == 13:
            length = data[pos] + 13
            pos += 1
        elif length == 14:
            length = ((data[pos] << 8) | data[pos + 1]) + 269
            pos += 2
        number += delta
        value = data[pos:pos + length]
        pos += length
        options.append((number, value))
    return options


def coap_validate_registration(data, payload):
    paths = []
    queries = set()
    for number, value in coap_parse_options(data):
        if number == 11:
            paths.append(value.decode("ascii", errors="replace"))
        elif number == 15:
            queries.add(value.decode("ascii", errors="replace"))

    required_queries = {"lwm2m=1.0", f"ep={LWM2M_ENDPOINT}", "lt=20", "b=U"}
    missing = required_queries - queries
    if paths != ["rd"] or missing:
        raise ValueError(f"bad LwM2M registration path={paths} queries={sorted(queries)} missing={sorted(missing)}")
    if b"</3/0>" not in payload:
        raise ValueError(f"bad LwM2M registration object links payload={payload!r}")


def coap_send_ack(sock, addr, request, code, payload=b""):
    token_len = request[0] & 0x0F
    mid = request[2:4]
    token = request[4:4 + token_len]
    resp = bytearray([0x60 | token_len, code]) + mid + token
    if payload:
        resp.append(0xFF)
        resp.extend(payload)
    sock.sendto(bytes(resp), addr)


def coap_add_uri_path(buf, path):
    first = True
    for segment in path.split("/"):
        data = segment.encode("ascii")
        delta = 11 if first else 0
        first = False
        buf.append((delta << 4) | len(data))
        buf.extend(data)


def coap_send_request(sock, addr, code, mid, path, payload=b""):
    req = bytearray([0x40, code, (mid >> 8) & 0xFF, mid & 0xFF])
    coap_add_uri_path(req, path)
    if payload:
        req.extend(b"\x11\x00")
        req.append(0xFF)
        req.extend(payload)
    sock.sendto(bytes(req), addr)


def coap_request_and_wait(state, addr, code, mid, path, payload, expected_codes, payload_contains=None):
    resp_queue = queue.Queue(maxsize=1)
    key = (addr, mid)
    with state.lock:
        state.pending[key] = resp_queue
    try:
        coap_send_request(state.sock, addr, code, mid, path, payload)
        try:
            resp = resp_queue.get(timeout=5)
        except queue.Empty as exc:
            raise TimeoutError(f"timeout waiting CoAP response mid=0x{mid:04x} path=/{path}") from exc
        resp_code = resp[1]
        resp_payload = coap_get_payload(resp)
        if resp_code not in expected_codes:
            raise ValueError(f"bad CoAP response code=0x{resp_code:02x} mid=0x{mid:04x} path=/{path}")
        if payload_contains is not None and payload_contains not in resp_payload:
            raise ValueError(f"bad CoAP payload mid=0x{mid:04x} path=/{path} payload={resp_payload!r}")
        print(
            f"[HOST_IOT_OTA_SERVER] builtin LwM2M ACK path=/{path} mid=0x{mid:04x} code=0x{resp_code:02x}",
            flush=True,
        )
    finally:
        with state.lock:
            state.pending.pop(key, None)


def coap_demo_device_driver(state, addr):
    time.sleep(1)
    print(f"[HOST_IOT_OTA_SERVER] builtin LwM2M demo READ /3/0/0 addr={addr}", flush=True)
    coap_request_and_wait(state, addr, 0x01, 0x7001, "3/0/0", b"", {0x45}, b"Open Mobile Alliance")
    time.sleep(1)
    print(f"[HOST_IOT_OTA_SERVER] builtin LwM2M demo WRITE /3/0/14 addr={addr}", flush=True)
    coap_request_and_wait(state, addr, 0x03, 0x7002, "3/0/14", b"+08:00", {0x44})
    time.sleep(1)
    print(f"[HOST_IOT_OTA_SERVER] builtin LwM2M demo EXECUTE /3/0/4 addr={addr}", flush=True)
    coap_request_and_wait(state, addr, 0x02, 0x7003, "3/0/4", b"", {0x44})


def coap_firmware_update_driver(state, addr):
    time.sleep(1)
    print(f"[HOST_IOT_OTA_SERVER] builtin LwM2M OTA WRITE /5/0/0 addr={addr}", flush=True)
    coap_request_and_wait(state, addr, 0x03, 0x7101, "5/0/0", PACKAGE, {0x44})
    time.sleep(1)
    print(f"[HOST_IOT_OTA_SERVER] builtin LwM2M OTA EXECUTE /5/0/2 addr={addr}", flush=True)
    coap_request_and_wait(state, addr, 0x02, 0x7102, "5/0/2", b"", {0x44})


def coap_server(host, port, enable_demo, enable_ota):
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((host, port))
        state = CoapState(sock)
        print(f"[HOST_IOT_OTA_SERVER] OTA manifest CoAP server listening on {host}:{port}", flush=True)
        while True:
            data, addr = sock.recvfrom(512)
            if len(data) < 4:
                continue
            code = data[1]
            payload = coap_get_payload(data)
            mid = coap_mid(data)
            with state.lock:
                pending = state.pending.get((addr, mid))
            if pending is not None:
                pending.put(data)
                continue
            if code == 0x02:
                print(f"[HOST_IOT_OTA_SERVER] builtin LwM2M REGISTER addr={addr} payload={payload!r}", flush=True)
                coap_validate_registration(data, payload)
                coap_send_ack(sock, addr, data, 0x41)
                if enable_demo and b"</3/0>" in payload and b"</5/0>" not in payload and addr not in state.demo_started:
                    state.demo_started.add(addr)
                    make_thread("builtin-lwm2m-demo-driver", coap_demo_device_driver, (state, addr)).start()
                if enable_ota and b"</5/0>" in payload and addr not in state.ota_started:
                    state.ota_started.add(addr)
                    make_thread("builtin-lwm2m-ota-driver", coap_firmware_update_driver, (state, addr)).start()
            elif code == 0x01:
                crc = zlib.crc32(PACKAGE) & 0xFFFFFFFF
                manifest = f"MANIFEST {VERSION} {len(PACKAGE)} {crc:08x}".encode("ascii")
                print(f"[HOST_IOT_OTA_SERVER] OTA manifest GET addr={addr}", flush=True)
                coap_send_ack(sock, addr, data, 0x45, manifest)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=None, help="Compatibility alias for --nb-iot-port")
    parser.add_argument("--nb-iot-port", type=int, default=56830)
    parser.add_argument("--lwm2m-port", type=int, default=5683)
    parser.add_argument("--lwm2m-manifest-port", type=int, default=56831)
    parser.add_argument("--leshan-web-port", type=int, default=8080)
    parser.add_argument("--leshan-jar", default=LESHAN_SERVER_JAR,
        help="Path to leshan-server-demo jar-with-dependencies. Defaults to LESHAN_SERVER_JAR or tools/leshan-server-demo.jar")
    parser.add_argument("--mqtt-port", type=int, default=1883)
    parser.add_argument("--mqtt-backend", choices=("auto", "mosquitto", "builtin"), default="auto")
    parser.add_argument("--lwm2m-backend", choices=("builtin", "leshan"), default="builtin")
    parser.add_argument("--target", action="append",
        choices=("all", "ota", "lwm2m", "mqtt", "nb_iot"), default=None,
        help="Enabled feature. Can be repeated. Default: all.")
    args = parser.parse_args()
    if args.port is not None:
        args.nb_iot_port = args.port

    targets = set(args.target or ["all"])
    if "all" in targets:
        targets.update(("ota", "lwm2m", "mqtt", "nb_iot"))
    enable_nb_iot = "nb_iot" in targets
    enable_lwm2m_demo = "lwm2m" in targets
    enable_mqtt_demo = "mqtt" in targets
    enable_lwm2m_ota = "ota" in targets and "lwm2m" in targets
    enable_mqtt_ota = "ota" in targets and "mqtt" in targets

    threads = []
    if enable_nb_iot:
        threads.append(make_thread("nb-iot-proxy", nb_iot_proxy_server, (args.host, args.nb_iot_port)))
    if enable_lwm2m_demo or enable_lwm2m_ota:
        threads.extend(start_lwm2m_backend(args.host, args.lwm2m_port, args.lwm2m_manifest_port,
            args.leshan_web_port, args.lwm2m_backend, args.leshan_jar, enable_lwm2m_demo, enable_lwm2m_ota))
    if enable_mqtt_demo or enable_mqtt_ota:
        threads.extend(start_mqtt_backend(args.host, args.mqtt_port, args.mqtt_backend,
            enable_mqtt_demo, enable_mqtt_ota))
    for thread in threads:
        thread.start()
    while True:
        try:
            name, exc = THREAD_FAILURES.get(timeout=1)
            raise RuntimeError(f"host helper thread {name} failed") from exc
        except queue.Empty:
            pass


if __name__ == "__main__":
    main()
