/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2024-09-10
 * Description: opcua client
 */

#include <stdlib.h>
#include <open62541/types.h>
#include <open62541/client.h>
#include <open62541/client_highlevel.h>
#include <open62541/server_config_default.h>
#include <open62541/plugin/log_stdout.h>

static void opcua_client_systime_read(UA_Client *client)
{
    /* Read the value attribute of the node. UA_Client_readValueAttribute is a
    * wrapper for the raw read service available as UA_Client_Service_read. */
    UA_Variant value; /* Variants can hold scalar values and arrays of any type */
    UA_Variant_init(&value);
    
    /* NodeId of the variable holding the current time */
    const UA_NodeId nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME);
    UA_StatusCode retval = UA_Client_readValueAttribute(client, nodeId, &value);
    
    if(retval == UA_STATUSCODE_GOOD && UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_DATETIME])) {
        UA_DateTime raw_date = *(UA_DateTime *)value.data;
        UA_DateTimeStruct dts = UA_DateTime_toStruct(raw_date);
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "date is: %u-%u-%u %u:%u:%u.%03u\n",
            dts.day, dts.month, dts.year, dts.hour, dts.min, dts.sec, dts.milliSec);
    } else {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                    "Reading the value failed with status code %s",
                    UA_StatusCode_name(retval));
    }
    
    /* Clean up */
    UA_Variant_clear(&value);
}

static void opcua_client_variable_read_write(UA_Client *client)
{
    UA_Variant value; 
    UA_Variant_init(&value);
    const UA_NodeId nodeId = UA_NODEID_STRING(1, "the.answer");
    UA_StatusCode retval = UA_Client_readValueAttribute(client, nodeId, &value);

    if(retval == UA_STATUSCODE_GOOD && UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32])) {
        UA_Int32 variableValue = *(UA_Int32 *)value.data;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Variable Value is: %d\n", variableValue);
    }

    UA_Int32 change = 100;
    UA_Variant newValue;
    UA_Variant_init(&newValue);
	UA_Variant_setScalar(&newValue, &change, &UA_TYPES[UA_TYPES_INT32]);
    retval = UA_Client_writeValueAttribute(client, nodeId, &newValue);

    if (retval == UA_STATUSCODE_GOOD) {	
    	retval = UA_Client_readValueAttribute(client, nodeId, &value);
    	if(retval == UA_STATUSCODE_GOOD && UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32])) {
        	UA_Int32 variableValue = *(UA_Int32 *)value.data;
        	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "New Variable Value is: %d\n", variableValue);
    	}
    }

    UA_Variant_clear(&value);
}

static void opcua_client_call_method(UA_Client *client)
{
    /* Call a remote method */
    UA_Variant input;
    UA_String argString = UA_STRING("everyone");
    UA_Variant_init(&input);
    UA_Variant_setScalarCopy(&input, &argString, &UA_TYPES[UA_TYPES_STRING]);
    size_t outputSize;
    UA_Variant *output;
    UA_StatusCode retval = UA_Client_call(client, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                            UA_NODEID_NUMERIC(1, 62541), 1, &input, &outputSize, &output);

    if(retval == UA_STATUSCODE_GOOD) {
        printf("Method call was successful, and %lu returned values available.\n",
               (unsigned long)outputSize);
        UA_String outString = *(UA_String*)output->data;
        printf("%.*s\n", outString.length, outString.data);
        UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
    } else {
        printf("Method call was unsuccessful, and %x returned values available.\n", retval);
    }

    UA_Variant_clear(&input);
}

void opcua_client_start(void)
{
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://localhost:4840");
    if(retval != UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
                    "The connection failed with status code %s",
                    UA_StatusCode_name(retval));
        UA_Client_delete(client);
        return;
    }

    /* read systime from opc ua server */
    opcua_client_systime_read(client);

    /* read and write variable from opc ua server */
    opcua_client_variable_read_write(client);

    /* call method defined in opc ua server */
    opcua_client_call_method(client);

    UA_Client_delete(client);
}
