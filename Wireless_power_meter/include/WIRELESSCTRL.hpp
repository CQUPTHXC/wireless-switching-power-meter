/*
 * @LastEditors: qingmeijiupiao
 * @Description: 无线控制相关部分
 * @Author: qingmeijiupiao
 * @Date: 2024-10-13 17:22:46
 */

#ifndef WIRELESSCTRL_HPP
#define WIRELESSCTRL_HPP
#include <Arduino.h>
#include "static/POWERMETER.hpp"
#include "static/powerctrl.hpp"
#include "NVSSTORAGE.hpp"
#include "static/ESPNOW.hpp"
extern POWERCTRL_t power_output;
uint8_t self_Macaddress[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};  // 自己的mac地址,初始化为广播地址
// 远程控制相关
namespace WIRELESSCTRL {
    
    void pair_func(data_package receive_data){

        NVSSTORAGE::pair_mac[0]=receive_data.data[0];
        NVSSTORAGE::pair_mac[1]=receive_data.data[1];
        NVSSTORAGE::pair_mac[2]=receive_data.data[2];
        NVSSTORAGE::pair_mac[3]=receive_data.data[3];
        NVSSTORAGE::pair_mac[4]=receive_data.data[4];
        NVSSTORAGE::pair_mac[5]=receive_data.data[5];
        NVSSTORAGE::NVS_save();
        memcpy(peerInfo.peer_addr, NVSSTORAGE::pair_mac, 6);
        esp_now_add_peer(&peerInfo);
        esp_now_send_package("pair",self_Macaddress,6,broadcastMacAddress);
    }

    // 心跳控制相关
    int Heartbeat_var=0;
    TaskHandle_t Heartbeat_task_handle = nullptr;
    int HeartbeatFRC = 3;
    int8_t Heartbeat_max_err=5;


    // 心跳任务
    void Heartbeat_task(void * pvParameters) {
        while (true) {
            if(Heartbeat_var>Heartbeat_max_err*1000/HeartbeatFRC){//心跳超时
                power_output.off();// 关闭电源
                Heartbeat_task_handle=nullptr;// 清空心跳任务
                vTaskDelete(NULL);// 删除心跳任务
            }
            delay(20);// 延时
            Heartbeat_var+=20;// 增加心跳
        }
    }
    // 心跳控制回调
    void Heartbeat_func(data_package receive_data){
        
        Heartbeat_var=0;// 重置心跳
        if(!power_output.getstate()){
            power_output.on();// 打开电源
        };
        if(Heartbeat_task_handle==nullptr){// 创建心跳任务
            xTaskCreate(Heartbeat_task, "Heartbeat_task", 8192, NULL, 5, &Heartbeat_task_handle);
        }
        uint8_t data[8];
        memcpy(data, receive_data.data,receive_data.data_len);
        int frc=*(int*)data;
        int max_err=*(int*)(data+4);
        if(frc!=0&&frc!=HeartbeatFRC){
            HeartbeatFRC=frc;
        }
        if(max_err!=-1&&max_err!=Heartbeat_max_err){
            Heartbeat_max_err=max_err;
        }
        
    }
    // 关闭心跳控制
    void close_heartbeat(data_package receive_data){
        if(Heartbeat_task_handle!=nullptr){
            vTaskDelete(Heartbeat_task_handle);
            Heartbeat_task_handle = nullptr;
        }
    }
    // 电源主动控制
    void power_ctrl(data_package receive_data) {
        bool state = *(bool*)receive_data.data;
        if (state) {
            if(!power_output.getstate())
                power_output.on();
        } else {
            if(power_output.getstate())
                power_output.off();
        }
    }
    // 获取电源输出状态
    void send_power_state(data_package receive_data) {
        bool state=power_output.getstate();
        esp_now_send_package("Power_state",(uint8_t*)(&state),1,NVSSTORAGE::pair_mac);
    }

    // 发送数据结构体
    struct Power_data{
        bool now_state;
        float voltage;
        float current;
        float mah;
        float mwh;
        uint32_t runtime;
    };
    int send_data_frc=10;
    TaskHandle_t send_data_task_handle = nullptr;
    void send_data_task( void * pvParameters ) {
        while (true) {
            Power_data send_power_data_package={
                power_output.getstate(),
                POWERMETER::voltage,
                POWERMETER::current,
                POWERMETER::output_mah,
                POWERMETER::output_mwh,
                millis()
            };
            uint8_t send_power_data_package_array[sizeof(Power_data)];
            memcpy(send_power_data_package_array, &send_power_data_package, sizeof(Power_data));
            esp_now_send_package("Power_data",send_power_data_package_array,sizeof(Power_data),NVSSTORAGE::pair_mac);
            delay(1000/send_data_frc);
        }
    }
    // 控制发送
    void send_data_ctrl(data_package receive_data) {
        uint8_t data[5];
        memcpy(data, receive_data.data,receive_data.data_len);
        bool is_continue = *(bool*)data;
        int frc=*(int*)(data+1);
        if(frc!=0){
            send_data_frc=frc;
        }
        if(is_continue&&send_data_task_handle==nullptr){
            xTaskCreatePinnedToCore(send_data_task, "send_data_task", 2048, NULL, 5, &send_data_task_handle, 1);
        }else{
            if(send_data_task_handle!=nullptr){
                vTaskDelete(send_data_task_handle);
                send_data_task_handle = nullptr;
            }
            uint32_t run_time=millis();
            Power_data send_power_data_package={
                power_output.getstate(),
                POWERMETER::voltage,
                POWERMETER::current,
                POWERMETER::output_mah,
                POWERMETER::output_mwh,
                run_time,
            };
            uint8_t send_power_data_package_array[sizeof(Power_data)];
            memcpy(send_power_data_package_array,&send_power_data_package,sizeof(Power_data));
            esp_now_send_package("Power_data",send_power_data_package_array,sizeof(Power_data),NVSSTORAGE::pair_mac);
        }
    }
    // 无线控制初始化
    void wireless_ctrl_setup() {
        WiFi.macAddress(self_Macaddress);// 获取自己的mac
        esp_now_setup(NVSSTORAGE::pair_mac);// ESP-NOW初始化
        // 注册回调函数
        callback_map["Heartbeatpackage"]=Heartbeat_func;
        callback_map["pair"]=pair_func;
        callback_map["close_heartbeat"]=close_heartbeat;
        callback_map["power_ctrl"]=power_ctrl;
        callback_map["get_power_state"]=send_power_state;
        callback_map["send_data_ctrl"]=send_data_ctrl;
        
    }

}
#endif