/* Plugin for exporting C++11 random number functionality
*Exports functions for random number generation
*Functions:
- seedRNG(seed)
- rollInt(min, max)
- rollDouble(min, max)
- rollNormal(mean, std_deviation)
- rollBool(chance_for_true)
- resetIndexRolls(string, array_length)  --String identifies the instance of SimpleNumDistribution to reset
- rollIndex(string, array_length)        --String identifies the instance of SimpleNumDistribution to use
                                         --(Shuffles a vector of indices, Next() increments through then reshuffles when end() is reached)
Author:  Josh Cooper
Created: Dec. 13 2017
Updated: Dec. 21 2017
SFMT-based reimplimentation by Trickerer
Aug. 21 2020
*/

/*
 * Copyright notice
 * ================
 * GNU General Public License http://www.gnu.org/licenses/gpl.html
 * This C++ implementation of SFMT contains parts of the original C code
 * which was published under the following BSD license, which is therefore
 * in effect in addition to the GNU General Public License.
 * Copyright (c) 2006, 2007 by Mutsuo Saito, Makoto Matsumoto and Hiroshima University.
 * Copyright (c) 2008 by Agner Fog.
 * Copyright (c) 2008-2013 Trinity Core
 * 
 *  BSD License:
 *  Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *     > Redistributions of source code must retain the above copyright notice, 
 *       this list of conditions and the following disclaimer.
 *     > Redistributions in binary form must reproduce the above copyright notice, 
 *       this list of conditions and the following disclaimer in the documentation
 *       and/or other materials provided with the distribution.
 *     > Neither the name of the Hiroshima University nor the names of its 
 *       contributors may be used to endorse or promote products derived from 
 *       this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "common.h"

// SFMT
#include <emmintrin.h>                 // Define SSE2 intrinsics
#include <time.h>
#include <new>

//#define MEXP 11213
  #define MEXP 19937

#if MEXP == 19937
#define SFMT_N    156                  // Size of state vector
#define SFMT_M    122                  // Position of intermediate feedback
#define SFMT_SL1   18                  // Left shift of W[N-1], 32-bit words
#define SFMT_SL2	  1                  // Left shift of W[0], *8, 128-bit words
#define SFMT_SR1   11                  // Right shift of W[M], 32-bit words
#define SFMT_SR2	  1                  // Right shift of W[N-2], *8, 128-bit words
#define SFMT_MASK	  0xdfffffef,0xddfecb7f,0xbffaffff,0xbffffff6 // AND mask
#define SFMT_PARITY 1,0,0,0x13c9e684   // Period certification vector

#elif MEXP == 11213
#define SFMT_N    88                   // Size of state vector
#define SFMT_M    68                   // Position of intermediate feedback
#define SFMT_SL1	14                   // Left shift of W[N-1], 32-bit words
#define SFMT_SL2	 3                   // Left shift of W[0], *8, 128-bit words
#define SFMT_SR1	 7                   // Right shift of W[M], 32-bit words
#define SFMT_SR2	 3                   // Right shift of W[N-2], *8, 128-bit words
#define SFMT_MASK	 0xeffff7fb,0xffffffef,0xdfdfbfff,0x7fffdbfd // AND mask
#define SFMT_PARITY 1,0,0xe8148000,0xd0c7afa3 // Period certification vector
#endif

static inline __m128i sfmt_recursion(__m128i const &a, __m128i const &b, 
__m128i const &c, __m128i const &d, __m128i const &mask) {
    __m128i a1, b1, c1, d1, z1, z2;
    b1 = _mm_srli_epi32(b, SFMT_SR1);
    a1 = _mm_slli_si128(a, SFMT_SL2);
    c1 = _mm_srli_si128(c, SFMT_SR2);
    d1 = _mm_slli_epi32(d, SFMT_SL1);
    b1 = _mm_and_si128(b1, mask);
    z1 = _mm_xor_si128(a, a1);
    z2 = _mm_xor_si128(b1, d1);
    z1 = _mm_xor_si128(z1, c1);
    z2 = _mm_xor_si128(z1, z2);
    return z2;
}

class SFMTRand
{
public:
    SFMTRand()
    {
        LastInterval = 0;
        RandomInit((int)(time(0)));
    }
    SFMTRand(int seed)
    {
        LastInterval = 0;
        RandomInit(seed ? seed : (int)(time(0)));
    }

    void RandomInit(int seed)                     // Re-seed
    {
        _seed = seed;
        uint32_t i;                         // Loop counter
        uint32_t y = seed;                  // Temporary
        uint32_t statesize = SFMT_N*4;      // Size of state vector
        // Fill state vector with random numbers from seed
        ((uint32_t*)state)[0] = y;
        static const uint32_t factor = 1812433253U;// Multiplication factor
        for (i = 1; i < statesize; i++) {
            y = factor * (y ^ (y >> 30)) + i;
            ((uint32_t*)state)[i] = y;
        }
        Init2();
    }
    int32_t IRandom(int32_t min, int32_t max)
    {
        // Slightly inaccurate if (max-min+1) is not a power of 2
        if (max <= min) {
            if (max == min) return min; else return 0x80000000;
        }
        uint32_t interval;                  // Length of interval
        uint64_t longran;                   // Random bits * interval
        uint32_t iran;                      // Longran / 2^32
        interval = (uint32_t)(max - min + 1);
        longran  = (uint64_t)BRandom() * interval;
        iran = (uint32_t)(longran >> 32);
        return (int32_t)iran + min;
    }
    uint32_t URandom(uint32_t min, uint32_t max)
    {
        // Slightly inaccurate if (max-min+1) is not a power of 2
        if (max <= min) {
            if (max == min) return min; else return 0;
        }
        uint32_t interval;                  // Length of interval
        uint64_t longran;                   // Random bits * interval
        uint32_t iran;                      // Longran / 2^32
        interval = (uint32_t)(max - min + 1);
        longran  = (uint64_t)BRandom() * interval;
        iran = (uint32_t)(longran >> 32);
        return iran + min;
    }
    double Random()
    {
        if (ix >= SFMT_N*4-1) {
            // Make sure we have at least two 32-bit numbers
            Generate();
        }
        uint64_t r = *(uint64_t*)((uint32_t*)state+ix);
        ix += 2;
        // 52 bits resolution for compatibility with assembly version:
        return (int64_t)(r >> 12) * (1./(67108864.0*67108864.0));
    }
    int get_seed() const { return _seed; }
private:
    uint32_t BRandom()                            // Output random bits
    {
        // Output 32 random bits
        uint32_t y;
        if (ix >= SFMT_N*4) { Generate(); }
        y = ((uint32_t*)state)[ix++];
        return y;
    }
    void Init2()   // Various initializations and period certification
    {
        uint32_t i, j, temp;
        // Initialize mask
        static const uint32_t maskinit[4] = {SFMT_MASK};
        mask = _mm_loadu_si128((__m128i*)maskinit);
        // Period certification
        // Define period certification vector
        static const uint32_t parityvec[4] = {SFMT_PARITY};
        // Check if parityvec & state[0] has odd parity
        temp = 0;
        for (i = 0; i < 4; i++)
            temp ^= parityvec[i] & ((uint32_t*)state)[i];
        for (i = 16; i > 0; i >>= 1) temp ^= temp >> i;
        if (!(temp & 1)) {
            // parity is even. Certification failed
            // Find a nonzero bit in period certification vector
            for (i = 0; i < 4; i++) {
                if (parityvec[i]) {
                    for (j = 1; j; j <<= 1) {
                        if (parityvec[i] & j) {
                            // Flip the corresponding bit in state[0] to change parity
                            ((uint32_t*)state)[i] ^= j;
                            // Done. Exit i and j loops
                            i = 5;  break;
                        }
                    }
                }
            }
        }
        // Generate first random numbers and set ix = 0
        Generate();
    }
    void Generate()  // Fill state array with new random numbers
    {
        int i;
        __m128i r, r1, r2;
        r1 = state[SFMT_N - 2];
        r2 = state[SFMT_N - 1];
        for (i = 0; i < SFMT_N - SFMT_M; i++) {
            r = sfmt_recursion(state[i], state[i + SFMT_M], r1, r2, mask);
            state[i] = r;
            r1 = r2;
            r2 = r;
        }
        for (; i < SFMT_N; i++) {
            r = sfmt_recursion(state[i], state[i + SFMT_M - SFMT_N], r1, r2, mask);
            state[i] = r;
            r1 = r2;
            r2 = r;
        }
        ix = 0;
    }
    void* operator new(size_t size, std::nothrow_t const&)   { return _mm_malloc(size, 16); }
    void operator delete(void* ptr, std::nothrow_t const&)   { _mm_free(ptr); }
    void* operator new(size_t size)                          { return _mm_malloc(size, 16); }
    void operator delete(void* ptr)                          { _mm_free(ptr); }
    void* operator new[](size_t size, std::nothrow_t const&) { return _mm_malloc(size, 16); }
    void operator delete[](void* ptr, std::nothrow_t const&) { _mm_free(ptr); }
    void* operator new[](size_t size)                        { return _mm_malloc(size, 16); }
    void operator delete[](void* ptr)                        { _mm_free(ptr); }

    __m128i  mask;                                // AND mask
    __m128i  state[SFMT_N];                       // State vector for SFMT generator
    uint32_t ix;                                  // Index into state array
    uint32_t LastInterval;                        // Last interval length for IRandom
    uint32_t RLimit;                              // Rejection limit used by IRandom
    int _seed;
};
// END SFMT

#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <hash_map>

#include "Error.h"
#include "Core.h"
#include "DataFuncs.h"
#include "Console.h"
#include "Export.h"
#include "PluginManager.h"

using namespace DFHack;
DFHACK_PLUGIN("cxxrandom");
#define PLUGIN_VERSION 2.0
color_ostream *cout = NULL;

DFhackCExport command_result plugin_init (color_ostream &out, std::vector <PluginCommand> &commands)
{
    cout = &out;
    return CR_OK;
}

DFhackCExport command_result plugin_shutdown (color_ostream &out)
{
    return CR_OK;
}

DFhackCExport command_result plugin_onstatechange(color_ostream &out, state_change_event event)
{
    return CR_OK;
}


class EnginesKeeper
{
private:
    EnginesKeeper() { counter = 0; }
    stdext::hash_map<uint16_t, SFMTRand> m_engines;
    uint16_t counter;
public:
    static EnginesKeeper& Instance()
    {
        static EnginesKeeper instance;
        return instance;
    }
    uint16_t NewEngine( uint64_t seed )
    {
        SFMTRand engine( seed != 0 ? seed : (int)(time(0)) );
        m_engines[++counter] = engine;
        return counter;
    }
    void DestroyEngine( uint16_t id )
    {
        m_engines.erase( id );
    }
    void NewSeed( uint16_t id, uint64_t seed )
    {
        CHECK_INVALID_ARGUMENT( m_engines.find( id ) != m_engines.end() );
        m_engines[id].RandomInit( seed != 0 ? seed : (int)(time(0)) );
    }
    SFMTRand& RNG( uint16_t id )
    {
        CHECK_INVALID_ARGUMENT( m_engines.find( id ) != m_engines.end() );
        return m_engines[id];
    }
};


uint16_t GenerateEngine( uint64_t seed )
{
    return EnginesKeeper::Instance().NewEngine( seed );
}
void DestroyEngine( uint16_t id )
{
    EnginesKeeper::Instance().DestroyEngine( id );
}
void NewSeed( uint16_t id, uint64_t seed )
{
    EnginesKeeper::Instance().NewSeed( id, seed );
}


int             rollInt(uint16_t id, int min, int max)
{
    if (min > max) std::swap(min, max);
    return int(EnginesKeeper::Instance().RNG(id).IRandom(min, max));
}
unsigned int    rollUInt(uint16_t id, unsigned int min, unsigned int max)
{
    if (min > max) std::swap(min, max);
    return unsigned int(EnginesKeeper::Instance().RNG(id).URandom(min, max));
}
double          rollDouble(uint16_t id, double min, double max)
{
    if (min > max) std::swap(min, max);
    return double(EnginesKeeper::Instance().RNG(id).Random()* (max - min) + min);
}
double          rollNormal(uint16_t id, double /*mean*/, double /*stddev*/)
{
    return EnginesKeeper::Instance().RNG(id).Random();
}
// Slightly inaccurate if (max-min+1) is not a power of 2
#define CHANCE_MIN 0
#define CHANCE_MAX 4095
bool            rollBool(uint16_t id, float probability)
{
    if (probability <= 0.f) return false;
    if (probability > 100.f) return true;

    return (rollUInt(id, CHANCE_MIN, CHANCE_MAX)) < ((CHANCE_MAX / 100.f) * probability);
}


class NumberSequence
{
private:
    unsigned short m_position;
    std::vector<int64_t> m_numbers;
public:
    NumberSequence(){m_position = 0;}
    NumberSequence( int64_t start, int64_t end )
    {
        m_position = 0;
        for( int64_t i = start; i <= end; ++i )
        {
            m_numbers.push_back( i );
        }
    }
    void Add( int64_t num ) { m_numbers.push_back( num ); }
    void Reset()            { m_numbers.clear(); }
    int64_t Next()
    {
        if(m_position >= m_numbers.size())
        {
            m_position = 0;
        }
        return m_numbers[m_position++];
    }
    void Shuffle( uint16_t id )
    {
        srand((unsigned int)EnginesKeeper::Instance().RNG(id).get_seed());
        std::random_shuffle(m_numbers.begin(), m_numbers.end());
    }
    void Print()
    {
        //for( auto v : m_numbers )
        for (std::vector<int64_t>::const_iterator ci = m_numbers.begin(); ci != m_numbers.end(); ++ci)
        {
            cout->print( INT64FMT " ", *ci );
        }
    }
};

class SequenceKeeper
{
private:
    SequenceKeeper() { counter = 0; }
    stdext::hash_map<uint16_t, NumberSequence> m_sequences;
    uint16_t counter;
public:
    static SequenceKeeper& Instance()
    {
        static SequenceKeeper instance;
        return instance;
    }
    uint16_t MakeNumSequence( int64_t start, int64_t end )
    {
        m_sequences[++counter] = NumberSequence( start, end );
        return counter;
    }
    uint16_t MakeNumSequence()
    {
        m_sequences[++counter] = NumberSequence();
        return counter;
    }
    void DestroySequence( uint16_t id )
    {
        m_sequences.erase( id );
    }
    void AddToSequence( uint16_t id, int64_t num )
    {
        CHECK_INVALID_ARGUMENT( m_sequences.find( id ) != m_sequences.end() );
        m_sequences[id].Add( num );
    }
    void Shuffle( uint16_t id, uint16_t rng_id )
    {
        CHECK_INVALID_ARGUMENT( m_sequences.find( id ) != m_sequences.end() );
        m_sequences[id].Shuffle( rng_id );
    }
    int64_t NextInSequence( uint16_t id )
    {
        CHECK_INVALID_ARGUMENT( m_sequences.find( id ) != m_sequences.end() );
        return m_sequences[id].Next();
    }
    void PrintSequence( uint16_t id )
    {
        CHECK_INVALID_ARGUMENT( m_sequences.find( id ) != m_sequences.end() );
        m_sequences[id].Print();
    }
};


uint16_t MakeNumSequence( int64_t start, int64_t end )
{
    if( start == end )
    {
        return SequenceKeeper::Instance().MakeNumSequence();
    }
    return SequenceKeeper::Instance().MakeNumSequence( start, end );
}

void     DestroyNumSequence( uint16_t id )
{
    SequenceKeeper::Instance().DestroySequence( id );
}

void     AddToSequence( uint16_t id, int64_t num )
{
    SequenceKeeper::Instance().AddToSequence( id, num );
}

void     ShuffleSequence( uint16_t rngID, uint16_t id )
{
    SequenceKeeper::Instance().Shuffle( id, rngID );
}

int64_t  NextInSequence( uint16_t id )
{
    return SequenceKeeper::Instance().NextInSequence( id );
}

void DebugSequence( uint16_t id )
{
    SequenceKeeper::Instance().PrintSequence( id );
}


DFHACK_PLUGIN_LUA_FUNCTIONS {
    DFHACK_LUA_FUNCTION(GenerateEngine),
    DFHACK_LUA_FUNCTION(DestroyEngine),
    DFHACK_LUA_FUNCTION(NewSeed),
    DFHACK_LUA_FUNCTION(rollInt),
    DFHACK_LUA_FUNCTION(rollDouble),
    DFHACK_LUA_FUNCTION(rollNormal),
    DFHACK_LUA_FUNCTION(rollBool),
    DFHACK_LUA_FUNCTION(MakeNumSequence),
    DFHACK_LUA_FUNCTION(DestroyNumSequence),
    DFHACK_LUA_FUNCTION(AddToSequence),
    DFHACK_LUA_FUNCTION(ShuffleSequence),
    DFHACK_LUA_FUNCTION(NextInSequence),
    DFHACK_LUA_FUNCTION(DebugSequence),
    DFHACK_LUA_END
};
