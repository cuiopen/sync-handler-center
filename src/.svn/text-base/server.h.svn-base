/**
 * @file server.h
 * @brief
 * @author jerryshao jerryshao@taomee.com
 * @version
 * @date 2011-05-30
 */
#ifndef _H_SERVER_20110530_H_
#define _H_SERVER_20110530_H_
#include <stdint.h>

typedef struct
{
    uint16_t chnl_id;
    uint8_t chnl_type;
} __attribute__((packed)) chnl_key_t;

struct chnl_key_comp
{
    bool operator() (const chnl_key_t& lhs, const chnl_key_t& rhs) const
    {
        if (lhs.chnl_id < rhs.chnl_id)
        {
            return true;
        }
        else if (lhs.chnl_id == rhs.chnl_id)
        {
            if (lhs.chnl_type < rhs.chnl_type)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
};

typedef struct
{
    uint32_t cmd_id;
    std::string secure_ip;
} cmd_info_t;

#endif
