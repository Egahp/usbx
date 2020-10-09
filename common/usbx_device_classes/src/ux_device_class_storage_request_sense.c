/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** USBX Component                                                        */ 
/**                                                                       */
/**   Device Storage Class                                                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define UX_SOURCE_CODE


/* Include necessary system files.  */

#include "ux_api.h"
#include "ux_device_class_storage.h"
#include "ux_device_stack.h"


#if UX_SLAVE_CLASS_STORAGE_BUFFER_SIZE < UX_SLAVE_CLASS_STORAGE_REQUEST_SENSE_RESPONSE_LENGTH
#error UX_SLAVE_CLASS_STORAGE_BUFFER_SIZE too small, please check
#endif

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _ux_device_class_storage_request_sense              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function performs a request sense command.                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    storage                               Pointer to storage class      */ 
/*    endpoint_in                           Pointer to IN endpoint        */
/*    endpoint_out                          Pointer to OUT endpoint       */
/*    cbwcb                                 Pointer to CBWCB              */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _ux_device_class_storage_csw_send     Send CSW                      */ 
/*    _ux_device_stack_transfer_request     Transfer request              */ 
/*    _ux_utility_memory_copy               Copy memory                   */ 
/*    _ux_utility_memory_set                Set memory                    */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Device Storage Class                                                */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*  09-30-2020     Chaoqiong Xiao           Modified comment(s),          */
/*                                            optimized command logic,    */
/*                                            verified memset and memcpy  */
/*                                            cases,                      */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _ux_device_class_storage_request_sense(UX_SLAVE_CLASS_STORAGE *storage, ULONG lun, UX_SLAVE_ENDPOINT *endpoint_in,
                                            UX_SLAVE_ENDPOINT *endpoint_out, UCHAR * cbwcb)
{

UINT                    status;
UX_SLAVE_TRANSFER       *transfer_request;
UCHAR                   *sense_buffer;
UCHAR                   key, code, qualifier;


    UX_PARAMETER_NOT_USED(cbwcb);
    UX_PARAMETER_NOT_USED(endpoint_out);

    /* Obtain the pointer to the transfer request.  */
    transfer_request =  &endpoint_in -> ux_slave_endpoint_transfer_request;

    /* Obtain sense buffer.  */
    sense_buffer = transfer_request -> ux_slave_transfer_request_data_pointer;

    /* Ensure it is cleaned.  */
    _ux_utility_memory_set(sense_buffer, 0, UX_SLAVE_CLASS_STORAGE_REQUEST_SENSE_RESPONSE_LENGTH); /* Use case of memset is verified. */
    
    /* Initialize the response buffer with the error code.  */
    sense_buffer[UX_SLAVE_CLASS_STORAGE_REQUEST_SENSE_RESPONSE_ERROR_CODE] = 
                    UX_SLAVE_CLASS_STORAGE_REQUEST_SENSE_RESPONSE_ERROR_CODE_VALUE;

    /* Extract sense key, code, qualifier.  */
    key = UX_DEVICE_CLASS_STORAGE_SENSE_KEY(storage -> ux_slave_class_storage_lun[lun].
                                            ux_slave_class_storage_request_sense_status);
    code = UX_DEVICE_CLASS_STORAGE_SENSE_CODE(storage -> ux_slave_class_storage_lun[lun].
                                            ux_slave_class_storage_request_sense_status);
    qualifier = UX_DEVICE_CLASS_STORAGE_SENSE_QUALIFIER(storage -> ux_slave_class_storage_lun[lun].
                                            ux_slave_class_storage_request_sense_status);

    /* Initialize the response buffer with the sense key.  */
    sense_buffer[UX_SLAVE_CLASS_STORAGE_REQUEST_SENSE_RESPONSE_SENSE_KEY] = key;

    /* Initialize the response buffer with the code.  */
    sense_buffer[UX_SLAVE_CLASS_STORAGE_REQUEST_SENSE_RESPONSE_CODE] = code;

    /* Initialize the response buffer with the code qualifier.  */
    sense_buffer[UX_SLAVE_CLASS_STORAGE_REQUEST_SENSE_RESPONSE_CODE_QUALIFIER] = qualifier;

    /* If trace is enabled, insert this event into the trace buffer.  */
    UX_TRACE_IN_LINE_INSERT(UX_TRACE_DEVICE_CLASS_STORAGE_REQUEST_SENSE, storage, lun, 
                            key, code, UX_TRACE_DEVICE_CLASS_EVENTS, 0, 0)

    /* Initialize the response buffer with the additional length.  */
    sense_buffer[UX_SLAVE_CLASS_STORAGE_REQUEST_SENSE_RESPONSE_ADD_LENGTH] =  10;

    /* Send a data payload with the sense codes.  */
    _ux_device_stack_transfer_request(transfer_request, UX_SLAVE_CLASS_STORAGE_REQUEST_SENSE_RESPONSE_LENGTH,
                              UX_SLAVE_CLASS_STORAGE_REQUEST_SENSE_RESPONSE_LENGTH);
    
    /* Now we set the CSW with success.  */
    storage -> ux_slave_class_storage_csw_status = UX_SLAVE_CLASS_STORAGE_CSW_PASSED;
    status = UX_SUCCESS;

    /* Return completion status.  */    
    return(status);
}

