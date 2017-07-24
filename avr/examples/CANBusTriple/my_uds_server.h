#include <uds_server.h>

#include <uds_hook.h>
#include <uds_security.h>
#include <uds_server_config.h>

void uds_hook_tester_present(void)
{
    println("TeslaECU: Got Tester Present"); //while(true){}
    // Do nothing
}

enum uds_error uds_hook_reset(enum uds_ecu_reset_type reset_type,
                              bool response_required)
{
    println("TeslaECU: Got Reset"); //while(true){}
    // Do nothing
    return UDS_ERROR_CONDITIONS_NOT_CORRECT;
}

enum uds_error
uds_hook_session_transition(enum uds_session_type new_session_type)
{
    println("TeslaECU: DSC Session transition"); //while(true){}
    // Do nothing
    (void)new_session_type;
    return UDS_ERROR_OK;
}

char *flag =    "flag{UD5_0N_4VR_lolWHY?}";

enum uds_error uds_hook_read_data_by_id(uint16_t data_id, unsigned char *data,
                                        size_t *len)
{
    if (uds_server_security_level() == 0)
    {
        return UDS_ERROR_SECURITY_ACCESS_DENIED;
    }
    if (data_id != 0)
    {
        return UDS_ERROR_REQUEST_OUT_OF_RANGE;
    }
    println("TeslaECU: Data ID OK"); //while(true){}
    memcpy(data, flag, 24);
    *len = 24;
    return UDS_ERROR_OK;
}

unsigned char last_seed[UDS_SECURITY_SEED_LEN];

enum uds_error uds_security_get_seed(enum uds_security_level requested_level,
                                     unsigned char *seed)
{
    println("TeslaECU: Got SA seed request"); //while(true){}
    (void)requested_level;
    for(int i = 0; i < UDS_SECURITY_SEED_LEN; i++) {
		last_seed[i] = seed[i] = (char)random(256);
    }
    println("Sent seed");
    hexd(last_seed, 20);
    return UDS_ERROR_OK;
}

bool uds_security_check_key(enum uds_security_level requested_level,
                            const unsigned char *key, size_t key_len)
{
    println("TeslaECU: Got SA challenge response"); //while(true){}
    (void)requested_level;
    if (key_len != UDS_SECURITY_SEED_LEN)
    {
        println("Fail! length");
        return false;
    }
    for (size_t i = 0; i < key_len; ++i)
    {
        if (key[i] != (last_seed[i] ^ 0x53U))
        {
            println("Fail! code");
            return false;
        }
    }
    println("Success!");
    return true;
}
