#pragma once

#ifndef _CUSTOM_HASH_H_
#define _CUSTOM_HASH_H_

#include "common.h"
#include <xhash>

// custom types hashes
struct String24Hash
{
    size_t operator()(const std::string24& s) const
    {
        return stdext::hash_value(s.c_str());
    }
	bool operator()(const std::string24& s1, const std::string24& s2) const
	{
	    return s1 == s2;
	}
	enum
	{
	    bucket_size = 4,
	    min_buckets = 8
    };
};

#endif // _CUSTOM_HASH_H_
