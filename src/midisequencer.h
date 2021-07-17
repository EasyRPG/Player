/*
Copyright (c) 2003-2006 yuno
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ''AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef midisequencer_h
#define midisequencer_h

#include <stdint.h>
#include <cstdio>
#include <string>
#include <vector>
#include <chrono>

#define META_EVENT_ALL_NOTE_OFF 0x8888

namespace midisequencer{
    /*
    typedef unsigned long uint_least32_t;
    */
    struct midi_message{
        std::chrono::microseconds time;
        uint_least32_t message;
        int port;
        int track;
        uint_least32_t tempo;
    };

    class uncopyable{
    public:
        uncopyable(){}
    private:
        uncopyable(const uncopyable&);
        void operator=(const uncopyable&);
    };

    class output:uncopyable{
    public:
        virtual void midi_message(int port, uint_least32_t message) = 0;
        virtual void sysex_message(int port, const void* data, std::size_t size) = 0;
        virtual void meta_event(int type, const void* data, std::size_t size) = 0;
        virtual void reset() = 0;
    protected:
        ~output(){}
    };

    class sequencer:uncopyable{
    public:
        sequencer();
        void clear();
        void rewind();
        std::vector<midi_message>::iterator rewind_to_loop();
        bool is_at_end();
        bool load(void* fp, int(*fgetc)(void*));
        bool load(std::FILE* fp);
        int get_num_ports()const;
        std::chrono::microseconds get_total_time()const;
        std::string get_title()const;
        std::string get_copyright()const;
        std::string get_song()const;
        uint32_t get_division()const;
        void play(std::chrono::microseconds time, output* out);
        void set_time(std::chrono::microseconds time, output* out);
        std::chrono::microseconds get_start_skipping_silence();
    private:
        std::vector<midi_message> messages;
        std::vector<midi_message>::iterator position;
        std::vector<midi_message>::iterator loop_position;
        std::vector<std::string> long_messages;
        uint32_t division = 0;
        void load_smf(void* fp, int(*fgetc)(void*));
    };
}

#endif
