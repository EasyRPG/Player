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

// MIDI software synthesizer.
#ifndef midisynth_h
#define midisynth_h

#include <stdint.h>
#include <map>
#include <memory>
#include <vector>

namespace midisynth{
    /*
    typedef short int_least16_t;
    typedef unsigned short uint_least16_t;
    typedef long int_least32_t;
    typedef unsigned long uint_least32_t;
    typedef __int64 int_least64_t;
    */
    class channel;

    // System mode enumeration type.
    enum system_mode_t{ system_mode_default, system_mode_gm, system_mode_gm2, system_mode_gs, system_mode_xg };

    // Basic uncopyable class.
    class uncopyable{
    public:
        uncopyable(){}
    private:
        uncopyable(const uncopyable&){}
        void operator=(const uncopyable&){}
    };

    // Note. Sound notes related.
    class note:uncopyable{
    public:
        note(int assign_, int panpot_):assign(assign_), panpot(panpot_){}
        virtual ~note(){}
        int get_assign()const{ return assign; }
        int get_panpot()const{ return panpot; }
        virtual bool synthesize(int_least32_t* buf, std::size_t samples, float rate, int_least32_t left, int_least32_t right) = 0;
        virtual void note_off(int velocity) = 0;
        virtual void sound_off() = 0;
        virtual void set_frequency_multiplier(float value) = 0;
        virtual void set_tremolo(int depth, float freq) = 0;
        virtual void set_vibrato(float depth, float freq) = 0;
        virtual void set_damper(int value) = 0;
        virtual void set_sostenute(int value) = 0;
        virtual void set_freeze(int value) = 0;
    private:
        int assign;
        int panpot;
    };

    // Notes factory.
    // Creates the appropriate note to the note-on message.
    class note_factory:uncopyable{
    public:
        virtual note* note_on(int_least32_t program, int note, int velocity, float frequency_multiplier)=0;
    protected:
        ~note_factory(){}
    };

    // MIDI channel.
    class channel:uncopyable{
        enum{ NUM_NOTES = 128 };
    public:
        channel(note_factory* factory, int bank);
        ~channel();

        int synthesize(int_least32_t* out, std::size_t samples, float rate, int_least32_t master_volume, int master_balance);
        void reset_all_parameters();
        void reset_all_controller();
        void all_note_off();
        void all_sound_off();
        void all_sound_off_immediately();

        void note_off(int note, int velocity);
        void note_on(int note, int velocity);
        void polyphonic_key_pressure(int note, int value);
        void program_change(int value){ set_program(128 * bank + value); }
        void channel_pressure(int value);
        void pitch_bend_change(int value){ pitch_bend = value; update_frequency_multiplier(); }
        void control_change(int control, int value);
        void bank_select(int value);

        void set_bank(int value){ bank = value; }
        void set_program(int value){ program = value; }
        void set_panpot(int value){ panpot = value; }
        void set_volume(int value){ volume = value; }
        void set_expression(int value){ expression = value; }
        void set_pitch_bend_sensitivity(int value){ pitch_bend_sensitivity = value; update_frequency_multiplier(); }
        void set_modulation_depth(int value){ modulation_depth = value; update_modulation(); }
        void set_modulation_depth_range(int value){ modulation_depth_range = value; update_modulation(); }
        void set_damper(int value);
        void set_sostenute(int value);
        void set_freeze(int value);
        void set_fine_tuning(int value){ fine_tuning = value; update_frequency_multiplier(); }
        void set_coarse_tuning(int value){ coarse_tuning = value; update_frequency_multiplier(); }
        void set_RPN(int value){ RPN = value; NRPN = 0x3FFF; }
        void set_NRPN(int value){ NRPN = value; RPN = 0x3FFF; }
        void set_tremolo_frequency(float value){ tremolo_frequency = value; }
        void set_vibrato_frequency(float value){ vibrato_frequency = value; }
        void set_master_frequency_multiplier(float value){ master_frequency_multiplier = value; update_frequency_multiplier(); }
        void set_mute(bool mute_){ mute = mute_; }
        void set_system_mode(system_mode_t mode){ system_mode = mode; }
        void mono_mode_on(){ all_note_off(); mono = true; }
        void poly_mode_on(){ all_note_off(); mono = false; }

        int get_program()const{ return program; }
        int get_bank()const{ return bank; }
        int get_panpot()const{ return panpot; }
        int get_volume()const{ return volume; }
        int get_expression()const{ return expression; }
        int get_channel_pressure()const{ return pressure; }
        int get_pitch_bend()const{ return pitch_bend; }
        int get_pitch_bend_sensitivity()const{ return pitch_bend_sensitivity; }
        int get_modulation_depth()const{ return modulation_depth; }
        int get_modulation_depth_range()const{ return modulation_depth_range; }
        int get_damper()const{ return damper; }
        int get_sostenute()const{ return sostenute; }
        int get_freeze()const{ return freeze; }
        int get_fine_tuning()const{ return fine_tuning; }
        int get_coarse_tuning()const{ return coarse_tuning; }
        int get_RPN()const{ return RPN; }
        int get_NRPN()const{ return NRPN; }
        float get_tremolo_frequency()const{ return tremolo_frequency; }
        float get_vibrato_frequency()const{ return vibrato_frequency; }
        bool get_mute()const{ return mute; }
        bool get_mono_mode()const{ return mono; }

    private:
        struct NOTE{
            class note* note;
            int key;
            enum STATUS{
                NOTEON, NOTEOFF, SOUNDOFF
            }status;
            NOTE(class note* p, int key_):note(p),key(key_),status(NOTEON){}
        };
        std::vector<NOTE> notes;
        note_factory* factory;
        int default_bank;
        int program;
        int bank;
        int panpot;
        int volume;
        int expression;
        int pressure;
        int pitch_bend;
        int pitch_bend_sensitivity;
        int modulation_depth;
        int modulation_depth_range;
        int damper;
        int sostenute;
        int freeze;
        int fine_tuning;
        int coarse_tuning;
        int RPN;
        int NRPN;
        bool mono;
        bool mute;
        float tremolo_frequency;
        float vibrato_frequency;
        float frequency_multiplier;
        float master_frequency_multiplier;
        system_mode_t system_mode;

        int get_registered_parameter();
        void set_registered_parameter(int value);
        void update_frequency_multiplier();
        void update_modulation();
    };

    // MIDI synthesizer.
    class synthesizer:uncopyable{
        enum{ NUM_CHANNELS = 16 };
    public:
        synthesizer(note_factory* factory);

        channel* get_channel(int ch);

        int synthesize(int_least16_t* output, std::size_t samples, float rate);
        int synthesize_mixing(int_least32_t* output, std::size_t samples, float rate);
        void reset();
        void reset_all_parameters();
        void reset_all_controller();
        void all_note_off();
        void all_sound_off();
        void all_sound_off_immediately();

        void note_on(int channel, int note, int velocity){ get_channel(channel)->note_on(note, velocity); }
        void note_off(int channel, int note, int velocity){ get_channel(channel)->note_off(note, velocity); }
        void polyphonic_key_pressure(int channel, int note, int value){ get_channel(channel)->polyphonic_key_pressure(note, value); }
        void control_change(int channel, int control, int value){ get_channel(channel)->control_change(control, value); }
        void program_change(int channel, int program){ get_channel(channel)->program_change(program); }
        void channel_pressure(int channel, int value){ get_channel(channel)->channel_pressure(value); }
        void pitch_bend_change(int channel, int value){ get_channel(channel)->pitch_bend_change(value); }
        void sysex_message(const void* data, std::size_t size);
        void midi_event(int command, int param1, int param2);
        void midi_event(uint_least32_t message){ midi_event(message & 0xFF, (message >> 8) & 0x7F, (message >> 16) & 0x7F); }

        void set_main_volume(int value){ main_volume = value; }
        void set_master_volume(int value){ master_volume = value; }
        void set_master_balance(int value){ master_balance = value; }
        void set_master_fine_tuning(int value){ master_fine_tuning = value; update_master_frequency_multiplier(); }
        void set_master_coarse_tuning(int value){ master_coarse_tuning = value; update_master_frequency_multiplier(); }
        void set_system_mode(system_mode_t mode);

        int get_main_volume()const{ return main_volume; }
        int get_master_volume()const{ return master_volume; }
        int get_master_balance()const{ return master_balance; }
        int get_master_fine_tuning()const{ return master_fine_tuning; }
        int get_master_coarse_tuning()const{ return master_coarse_tuning; }
        system_mode_t get_system_mode()const{ return system_mode; }

    private:
        std::unique_ptr<channel> channels[NUM_CHANNELS];
        float active_sensing;
        int main_volume;
        int master_volume;
        int master_balance;
        int master_fine_tuning;
        int master_coarse_tuning;
        float master_frequency_multiplier;
        system_mode_t system_mode;
        void update_master_frequency_multiplier();
    };

    // Sine wave generator.
    // Generates a sine wave with an amplitude of 32768 (-32767...32767).
    class sine_wave_generator{
    public:
        sine_wave_generator();
        sine_wave_generator(float cycle);
        void set_cycle(float cycle);
        void add_modulation(int_least32_t x);
        int get_next();
        int get_next(int_least32_t modulation);
    private:
        uint_least32_t position;
        uint_least32_t step;
    };

    // Envelope generator.
    // Generates 0 to 32767 values when the TL = 0.
    class envelope_generator{
    public:
        envelope_generator(int AR, int DR, int SR, int RR, int SL, int TL);
        void set_rate(float rate);
        void set_hold(float value);
        void set_freeze(float value);
        void key_off();
        void sound_off();
        bool is_finished()const{ return state == FINISHED; }
        int get_next();
    private:
        enum{ ATTACK, ATTACK_RELEASE, DECAY, DECAY_RELEASE, SASTAIN, RELEASE, SOUNDOFF, FINISHED }state;
        int AR, DR, SR, RR, TL;
        uint_least32_t fAR, fDR, fSR, fRR, fSL, fTL, fOR, fSS, fDRR, fDSS;
        uint_least32_t current;
        float rate;
        float hold;
        float freeze;
        void update_parameters();
    };

    // FM operator (modulator and carrier).
    class fm_operator{
    public:
        fm_operator(int AR, int DR, int SR, int RR, int SL, int TL, int KS, int ML, int DT, int AMS, int key);
        void set_freq_rate(float freq, float rate);
        void set_hold(float value){ eg.set_hold(value); }
        void set_freeze(float value){ eg.set_freeze(value); }
        void add_modulation(int_least32_t x){ swg.add_modulation(x); }
        void key_off(){ eg.key_off(); }
        void sound_off(){ eg.sound_off(); }
        bool is_finished()const{ return eg.is_finished(); }
        int get_next();
        int get_next(int modulate);
        int get_next(int lfo, int modulate);
        inline int operator()(){ return get_next(); }
        inline int operator()(int m){ return get_next(m); }
        inline int operator()(int lfo, int m){ return get_next(lfo, m); }
    private:
        sine_wave_generator swg;
        envelope_generator eg;
        float ML;
        float DT;
        int_least32_t ams_factor;
        int_least32_t ams_bias;
    };

    // FM sound source parameters.
    struct FMPARAMETER{
        int ALG, FB, LFO;
        struct{
            int AR, DR, SR, RR, SL, TL, KS, ML, DT, AMS;
        }op1, op2, op3, op4;
    };
    // Drum parameters.
    struct DRUMPARAMETER:FMPARAMETER{
        int key, panpot, assign;
    };

    // FM sound generator.
    class fm_sound_generator{
    public:
        fm_sound_generator(const FMPARAMETER& params, int note, float frequency_multiplier);
        void set_rate(float rate);
        void set_frequency_multiplier(float value);
        void set_damper(int damper);
        void set_sostenute(int sostenute);
        void set_freeze(int freeze);
        void set_tremolo(int depth, float frequency);
        void set_vibrato(float depth, float frequency);
        void key_off();
        void sound_off();
        bool is_finished()const;
        int get_next();
    private:
        fm_operator op1;
        fm_operator op2;
        fm_operator op3;
        fm_operator op4;
        sine_wave_generator ams_lfo;
        sine_wave_generator vibrato_lfo;
        sine_wave_generator tremolo_lfo;
        int ALG;
        int FB;
        float freq;
        float freq_mul;
        float ams_freq;
        bool ams_enable;
        int tremolo_depth;
        float tremolo_freq;
        int vibrato_depth;
        float vibrato_freq;
        float rate;
        int feedback;
        int damper;
        int sostenute;
    };

    // FM sound generator notes.
    class fm_note:public note{
    public:
        fm_note(const FMPARAMETER& params, int note, int velocity, int panpot, int assign, float frequency_multiplier);
        virtual void release(){ delete this; }
        virtual bool synthesize(int_least32_t* buf, std::size_t samples, float rate, int_least32_t left, int_least32_t right);
        virtual void note_off(int velocity);
        virtual void sound_off();
        virtual void set_frequency_multiplier(float value);
        virtual void set_tremolo(int depth, float freq);
        virtual void set_vibrato(float depth, float freq);
        virtual void set_damper(int value);
        virtual void set_sostenute(int value);
        virtual void set_freeze(int value);
    public:
        fm_sound_generator fm;
        int velocity;
    };

    // FM sound generator note factory.
    class fm_note_factory:public note_factory{
    public:
        fm_note_factory();
        void clear();
        void get_program(int number, FMPARAMETER& p);
        bool set_program(int number, const FMPARAMETER& p);
        bool set_drum_program(int number, const DRUMPARAMETER& p);
        virtual note* note_on(int_least32_t program, int note, int velocity, float frequency_multiplier);
    private:
        std::map<int, FMPARAMETER> programs;
        std::map<int, DRUMPARAMETER> drums;
    };
}

#endif
