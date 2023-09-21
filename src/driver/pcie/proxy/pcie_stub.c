

/* 
1. unip 提供接口，由用户传入 {vendor_id device_id} 数组, 将该数组传递给 mcs_ko 
   mcs_ko 保存该数组，然后进行注册 dev_driver，返回OK
2. unip 提供接口，用于获取 bar 地址， mcs_ko 返回bar地址，
   unip 对bar地址进行映射，返回虚拟地址给用户
3. 中断注册，待定
*/

