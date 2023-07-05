#ifndef EP_MESSAGES_H
#define EP_MESSAGES_H

#include "connection.h"
#include "packet.h"
#include <memory>
#include <lcf/rpg/sound.h>
#include "../game_pictures.h"

namespace Messages {
	using EncodedPacket = Multiplayer::EncodedPacket;
	using DecodedPacket = Multiplayer::DecodedPacket;
	using ParameterList = Multiplayer::Connection::ParameterList;

	/**
	 * Heartbeat
	 */

	class HeartbeatEncodedPacket : public EncodedPacket {
	public:
		HeartbeatEncodedPacket(): EncodedPacket("hb") {}
		std::string ToBytes() const override { return Build(); }
	};
	class HeartbeatDecodedPacket : public DecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "hb" };
		HeartbeatDecodedPacket(const ParameterList& v) {}
	};

	/**
	 * Room
	 */

	class RoomEncodedPacket : public EncodedPacket {
	public:
		RoomEncodedPacket(int _room_id): EncodedPacket("room"), room_id(_room_id) {}
		std::string ToBytes() const override { return Build(room_id); }
	protected:
		int room_id;
	};
	class RoomDecodedPacket : public DecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "room" };
		RoomDecodedPacket(const ParameterList& v)
			: room_id(Decode<int>(v.at(0))) {}
		int room_id;
	};

	/**
	 * Base Class: Player
	 */

	class PlayerEncodedPacket : public EncodedPacket {
	public:
		PlayerEncodedPacket(std::string _name) // C2S
			: EncodedPacket(std::move(_name)) {}
		PlayerEncodedPacket(std::string _name, int _id) // S2C
			: EncodedPacket(std::move(_name)), id(_id) {}
		void Append(std::string& s) const { AppendPartial(s, id); }
	protected:
		int id{0};
	};
	class PlayerDecodedPacket : public DecodedPacket {
	public:
		PlayerDecodedPacket(std::string_view _id) : id(Decode<int>(_id)) {}
		const int id;
	};

	/**
	 * Connect
	 */

	class ConnectEncodedPacket : public PlayerEncodedPacket {
	public:
		ConnectEncodedPacket(int _id)
			: PlayerEncodedPacket("c", _id) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerEncodedPacket::Append(r);
			return r;
		}
	};
	class ConnectDecodedPacket : public PlayerDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "c" };
		ConnectDecodedPacket(const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)) {}
	};

	/**
	 * Disconnect
	 */

	class DisconnectEncodedPacket : public PlayerEncodedPacket {
	public:
		DisconnectEncodedPacket(int _id)
			: PlayerEncodedPacket("d", _id) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerEncodedPacket::Append(r);
			return r;
		}
	};
	class DisconnectDecodedPacket : public PlayerDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "d" };
		DisconnectDecodedPacket(const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)) {}
	};

	/**
	 * Nametag
	 */

	class NametagEncodedPacket : public PlayerEncodedPacket {
	public:
		NametagEncodedPacket(int _id, std::string _name)
			: PlayerEncodedPacket("ntag", _id), name(std::move(_name)) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerEncodedPacket::Append(r);
			AppendPartial(r, name);
			return r;
		}
	protected:
		std::string name;
	};
	class NametagDecodedPacket : public PlayerDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "ntag" };
		NametagDecodedPacket(const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)),
			name(v.at(1)) {}
		const std::string name;
	};

	/**
	 * Chat
	 */

	class ChatEncodedPacket : public PlayerEncodedPacket {
	public:
		ChatEncodedPacket(int _v, std::string _m) // C2S
			: PlayerEncodedPacket("say"), visibility(_v), msg(std::move(_m)) {}
		ChatEncodedPacket(int _id, int _v, std::string _m) // S2C
			: PlayerEncodedPacket("say", _id), visibility(_v), msg(std::move(_m)) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerEncodedPacket::Append(r);
			AppendPartial(r, visibility, msg);
			return r;
		};
	protected:
		int visibility;
		std::string msg;
	};
	class ChatDecodedPacket : public PlayerDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "say" };
		ChatDecodedPacket(const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)),
			visibility(Decode<int>(v.at(1))), msg(v.at(2)) {}
		const int visibility;
		const std::string msg;
	};

	/**
	 * Move
	 */

	class MainPlayerPosEncodedPacket : public EncodedPacket {
	public:
		MainPlayerPosEncodedPacket(int _x, int _y)
			: EncodedPacket("m"), x(_x), y(_y) {}
		std::string ToBytes() const override { return Build(x, y); }
	protected:
		int x, y;
	};
	class MainPlayerPosDecodedPacket : public DecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "m" };
		MainPlayerPosDecodedPacket(const ParameterList& v)
			: x(Decode<int>(v.at(0))), y(Decode<int>(v.at(1))) {}
		const int x, y;
	};

	class MoveEncodedPacket : public PlayerEncodedPacket {
	public:
		MoveEncodedPacket(int _id, int _x, int _y)
			: PlayerEncodedPacket("m", _id), x(_x), y(_y) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerEncodedPacket::Append(r);
			AppendPartial(r, x, y);
			return r;
		};
	protected:
		int x, y;
	};
	class MoveDecodedPacket : public PlayerDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "m" };
		MoveDecodedPacket(const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)),
			x(Decode<int>(v.at(1))), y(Decode<int>(v.at(2))) {}
		const int x, y;
	};

	/**
	 * Teleport
	 */

	class TeleportEncodedPacket : public EncodedPacket {
	public:
		TeleportEncodedPacket(int _x, int _y)
			: EncodedPacket("tp"), x(_x), y(_y) {}
		std::string ToBytes() const override { return Build(x, y); }
	protected:
		int x, y;
	};

	/**
	 * Jump
	 */

	class JumpEncodedPacket : public PlayerEncodedPacket {
	public:
		JumpEncodedPacket(int _x, int _y) // C2S
			: PlayerEncodedPacket("jmp"), x(_x), y(_y) {}
		JumpEncodedPacket(int _id, int _x, int _y) // S2C
			: PlayerEncodedPacket("jmp", _id), x(_x), y(_y) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerEncodedPacket::Append(r);
			AppendPartial(r, x, y);
			return r;
		};
	protected:
		int x, y;
	};
	class JumpDecodedPacket : public PlayerDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "jmp" };
		JumpDecodedPacket(const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)),
			x(Decode<int>(v.at(1))), y(Decode<int>(v.at(2))) {}
		const int x, y;
	};

	/**
	 * Facing
	 */

	class FacingEncodedPacket : public PlayerEncodedPacket {
	public:
		FacingEncodedPacket(int _d) // C2S
			: PlayerEncodedPacket("f"), d(_d) {}
		FacingEncodedPacket(int _id, int _d) // S2C
			: PlayerEncodedPacket("f", _id), d(_d) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerEncodedPacket::Append(r);
			AppendPartial(r, d);
			return r;
		};
	protected:
		int d;
	};
	class FacingDecodedPacket : public PlayerDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "f" };
		FacingDecodedPacket(const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)), facing(Decode<int>(v.at(1))) {}
		const int facing;
	};

	/**
	 * Speed
	 */

	class SpeedEncodedPacket : public PlayerEncodedPacket {
	public:
		SpeedEncodedPacket(int _spd) // C2S
			: PlayerEncodedPacket("spd"), spd(_spd) {}
		SpeedEncodedPacket(int _id, int _spd) // S2C
			: PlayerEncodedPacket("spd", _id), spd(_spd) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerEncodedPacket::Append(r);
			AppendPartial(r, spd);
			return r;
		};
	protected:
		int spd;
	};
	class SpeedDecodedPacket : public PlayerDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "spd" };
		SpeedDecodedPacket(const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)), speed(Decode<int>(v.at(1))) {}
		const int speed;
	};

	/**
	 * Sprite
	 */

	class SpriteEncodedPacket : public PlayerEncodedPacket {
	public:
		SpriteEncodedPacket(std::string _n, int _i) // C2S
			: PlayerEncodedPacket("spr"), name(_n), index(_i) {}
		SpriteEncodedPacket(int _id, std::string _n, int _i) // S2C
			: PlayerEncodedPacket("spr", _id), name(_n), index(_i) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerEncodedPacket::Append(r);
			AppendPartial(r, name, index);
			return r;
		};
	protected:
		std::string name;
		int index;
	};
	class SpriteDecodedPacket : public PlayerDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "spr" };
		SpriteDecodedPacket(const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)),
			name(v.at(1)), index(Decode<int>(v.at(2))) {}
		const std::string name;
		const int index;
	};

	/**
	 * Base Class: Flash
	 */

	class FlashEncodedPacket : public PlayerEncodedPacket {
	public:
		FlashEncodedPacket(int _r, int _g, int _b, int _p, int _f) // C2S
			: PlayerEncodedPacket("fl"), r(_r), g(_g), b(_b), p(_p), f(_f) {}
		FlashEncodedPacket(int _id, int _r, int _g, int _b, int _p, int _f) // S2C
			: PlayerEncodedPacket("fl", _id), r(_r), g(_g), b(_b), p(_p), f(_f) {}
		std::string ToBytes() const override {
			std::string res {GetName()};
			PlayerEncodedPacket::Append(res);
			AppendPartial(res, r, g, b, p, f);
			return res;
		};
	protected:
		int r;
		int g;
		int b;
		int p;
		int f;
	};
	class FlashDecodedPacket : public PlayerDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "fl" };
		FlashDecodedPacket(const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)),
			r(Decode<int>(v.at(1))),
			g(Decode<int>(v.at(2))),
			b(Decode<int>(v.at(3))),
			p(Decode<int>(v.at(4))),
			f(Decode<int>(v.at(5))) {}
		const int r;
		const int g;
		const int b;
		const int p;
		const int f;
	};

	/**
	 * Repeating Flash
	 */

	class RepeatingFlashEncodedPacket : public PlayerEncodedPacket {
	public:
		RepeatingFlashEncodedPacket(int _r, int _g, int _b, int _p, int _f) // C2S
			: PlayerEncodedPacket("rfl"), r(_r), g(_g), b(_b), p(_p), f(_f) {}
		RepeatingFlashEncodedPacket(int _id, int _r, int _g, int _b, int _p, int _f) // S2C
			: PlayerEncodedPacket("rfl", _id), r(_r), g(_g), b(_b), p(_p), f(_f) {}
		std::string ToBytes() const override {
			std::string res {GetName()};
			PlayerEncodedPacket::Append(res);
			AppendPartial(res, r, g, b, p, f);
			return res;
		};
	protected:
		int r;
		int g;
		int b;
		int p;
		int f;
	};
	class RepeatingFlashDecodedPacket : public FlashDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "rfl" };
		RepeatingFlashDecodedPacket(const ParameterList& v)
			: FlashDecodedPacket(v) {}
	};

	/**
	 * Remove Repeating Flash
	 */

	class RemoveRepeatingFlashEncodedPacket : public PlayerEncodedPacket {
	public:
		RemoveRepeatingFlashEncodedPacket() : PlayerEncodedPacket("rrfl") {} // C2S
		RemoveRepeatingFlashEncodedPacket(int _id) : PlayerEncodedPacket("rrfl", _id) {} // S2C
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerEncodedPacket::Append(r);
			return r;
		};
	};
	class RemoveRepeatingFlashDecodedPacket : public PlayerDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "rrfl" };
		RemoveRepeatingFlashDecodedPacket(const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)) {}
	};

	/**
	 * Hidden
	 */

	class HiddenEncodedPacket : public PlayerEncodedPacket {
	public:
		HiddenEncodedPacket(int _hidden_bin) // C2S
			: PlayerEncodedPacket("h"), hidden_bin(_hidden_bin) {}
		HiddenEncodedPacket(int _id, int _hidden_bin) // S2C
			: PlayerEncodedPacket("h", _id), hidden_bin(_hidden_bin) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerEncodedPacket::Append(r);
			AppendPartial(r, hidden_bin);
			return r;
		};
	protected:
		int hidden_bin;
	};
	class HiddenDecodedPacket : public PlayerDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "h" };
		HiddenDecodedPacket(const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)),
			hidden_bin(Decode<int>(v.at(1))) {}
		const int hidden_bin;
	};

	/**
	 * System Graphic
	 */

	class SysNameEncodedPacket : public PlayerEncodedPacket {
	public:
		SysNameEncodedPacket(std::string _s) // C2S
			: PlayerEncodedPacket("sys"), s(std::move(_s)) {}
		SysNameEncodedPacket(int _id, std::string _s) // S2C
			: PlayerEncodedPacket("sys", _id), s(std::move(_s)) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerEncodedPacket::Append(r);
			AppendPartial(r, s);
			return r;
		};
	protected:
		std::string s;
	};
	class SystemDecodedPacket : public PlayerDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "sys" };
		SystemDecodedPacket(const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)), name(v.at(1)) {}
		const std::string name;
	};

	/**
	 * Sound Effect
	 */

	class SEEncodedPacket : public PlayerEncodedPacket {
	public:
		SEEncodedPacket(lcf::rpg::Sound _d) // C2S
			: PlayerEncodedPacket("se"), snd(std::move(_d)) {}
		SEEncodedPacket(int _id, lcf::rpg::Sound _d) // S2C
			: PlayerEncodedPacket("se", _id), snd(std::move(_d)) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerEncodedPacket::Append(r);
			AppendPartial(r, snd.name, snd.volume, snd.tempo, snd.balance);
			return r;
		};
	protected:
		lcf::rpg::Sound snd;
	};
	class SEDecodedPacket : public PlayerDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "se" };
		static lcf::rpg::Sound BuildSound(const ParameterList& v) {
			lcf::rpg::Sound s;
			s.name = v.at(1);
			s.volume = Decode<int>(v.at(2));
			s.tempo = Decode<int>(v.at(3));
			s.balance = Decode<int>(v.at(4));
			return s;
		}
		SEDecodedPacket(const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)), snd(BuildSound(v)) {}
		const lcf::rpg::Sound snd;
	};

	/**
	 * Base Class: Picture
	 */

	class PictureEncodedPacket : public PlayerEncodedPacket {
	public:
		PictureEncodedPacket(std::string _name, int _pic_id, Game_Pictures::Params& _p, // C2S
				int _mx, int _my, int _panx, int _pany)
			: PlayerEncodedPacket(std::move(_name)), pic_id(_pic_id), p(_p),
			map_x(_mx), map_y(_my), pan_x(_panx), pan_y(_pany) {}
		PictureEncodedPacket(std::string _name, int _id, int _pic_id, Game_Pictures::Params& _p, // S2C
				int _mx, int _my, int _panx, int _pany)
			: PlayerEncodedPacket(std::move(_name), _id), pic_id(_pic_id), p(_p),
			map_x(_mx), map_y(_my), pan_x(_panx), pan_y(_pany) {}
		void Append(std::string& s) const {
			PlayerEncodedPacket::Append(s);
			AppendPartial(s, pic_id, p.position_x, p.position_y,
					map_x, map_y, pan_x, pan_y,
					p.magnify, p.top_trans, p.bottom_trans,
					p.red, p.green, p.blue, p.saturation,
					p.effect_mode, p.effect_power);
		}
	protected:
		int pic_id;
		Game_Pictures::Params& p;
		int map_x, map_y;
		int pan_x, pan_y;
	};
	class PictureDecodedPacket : public PlayerDecodedPacket {
	public:
		static void BuildParams(Game_Pictures::Params& p, const ParameterList& v) {
			p.position_x = Decode<int>(v.at(2));
			p.position_y = Decode<int>(v.at(3));
			p.magnify = Decode<int>(v.at(8));
			p.top_trans = Decode<int>(v.at(9));
			p.bottom_trans = Decode<int>(v.at(10));
			p.red = Decode<int>(v.at(11));
			p.green = Decode<int>(v.at(12));
			p.blue = Decode<int>(v.at(13));
			p.saturation = Decode<int>(v.at(14));
			p.effect_mode = Decode<int>(v.at(15));
			p.effect_power = Decode<int>(v.at(16));
		}
		PictureDecodedPacket(Game_Pictures::Params& _params, const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)), params(_params),
			pic_id(Decode<int>(v.at(1))),
			map_x(Decode<int>(v.at(4))), map_y(Decode<int>(v.at(5))),
			pan_x(Decode<int>(v.at(6))), pan_y(Decode<int>(v.at(7))) {}
		Game_Pictures::Params& params;
		const int pic_id;
		int map_x, map_y;
		int pan_x, pan_y;
	};

	/**
	 * Show Picture
	 */

	class ShowPictureEncodedPacket : public PictureEncodedPacket {
	public:
		ShowPictureEncodedPacket(int _pid, Game_Pictures::ShowParams _p, // C2S
				int _mx, int _my, int _px, int _py)
			: PictureEncodedPacket("ap", _pid, p_show, _mx, _my, _px, _py), p_show(std::move(_p)) {}
		ShowPictureEncodedPacket(int _id, int _pid, Game_Pictures::ShowParams _p, // S2C
				int _mx, int _my, int _px, int _py)
			: PictureEncodedPacket("ap", _id, _pid, p_show, _mx, _my, _px, _py), p_show(std::move(_p)) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PictureEncodedPacket::Append(r);
			AppendPartial(r, p_show.name, p_show.use_transparent_color, p_show.fixed_to_map);
			return r;
		}
	protected:
		Game_Pictures::ShowParams p_show;
	};
	class ShowPictureDecodedPacket : public PictureDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "ap" };
		Game_Pictures::ShowParams BuildParams(const ParameterList& v) const {
			Game_Pictures::ShowParams p;
			PictureDecodedPacket::BuildParams(p, v);
			p.name = v.at(17);
			p.use_transparent_color = Decode<bool>(v.at(18));
			p.fixed_to_map = Decode<bool>(v.at(19));
			return p;
		}
		ShowPictureDecodedPacket(const ParameterList& v)
			: PictureDecodedPacket(params, v), params(BuildParams(v)) {}
		Game_Pictures::ShowParams params;
	};

	/**
	 * Move Picture
	 */

	class MovePictureEncodedPacket : public PictureEncodedPacket {
	public:
		MovePictureEncodedPacket(int _pid, Game_Pictures::MoveParams _p, // C2S
				int _mx, int _my, int _px, int _py)
			: PictureEncodedPacket("mp", _pid, p_move, _mx, _my, _px, _py), p_move(std::move(_p)) {}
		MovePictureEncodedPacket(int _id, int _pid, Game_Pictures::MoveParams _p, // S2C
				int _mx, int _my, int _px, int _py)
			: PictureEncodedPacket("mp", _id, _pid, p_move, _mx, _my, _px, _py), p_move(std::move(_p)) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PictureEncodedPacket::Append(r);
			AppendPartial(r, p_move.duration);
			return r;
		}
	protected:
		Game_Pictures::MoveParams p_move;
	};
	class MovePictureDecodedPacket : public PictureDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "mp" };
		Game_Pictures::MoveParams BuildParams(const ParameterList& v) const {
			Game_Pictures::MoveParams p;
			PictureDecodedPacket::BuildParams(p, v);
			p.duration = Decode<int>(v.at(17));
			return p;
		}
		MovePictureDecodedPacket(const ParameterList& v)
			: PictureDecodedPacket(params, v), params(BuildParams(v)) {}
		Game_Pictures::MoveParams params;
	};

	/**
	 * Erase Picture
	 */

	class ErasePictureEncodedPacket : public PlayerEncodedPacket {
	public:
		ErasePictureEncodedPacket(int _pid) : PlayerEncodedPacket("rp"), pic_id(_pid) {} // C2S
		ErasePictureEncodedPacket(int _id, int _pid) : PlayerEncodedPacket("rp", _id), pic_id(_pid) {} // S2C
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerEncodedPacket::Append(r);
			AppendPartial(r, pic_id);
			return r;
		}
	protected:
		int pic_id;
	};
	class ErasePictureDecodedPacket : public PlayerDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "rp" };
		ErasePictureDecodedPacket(const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)), pic_id(Decode<int>(v.at(1))) {}
		const int pic_id;
	};

	/**
	 * Show Player Battle Animation
	 */

	class ShowPlayerBattleAnimEncodedPacket : public PlayerEncodedPacket {
	public:
		ShowPlayerBattleAnimEncodedPacket(int _anim_id) // C2S
			: PlayerEncodedPacket("ba"), anim_id(_anim_id) {}
		ShowPlayerBattleAnimEncodedPacket(int _id, int _anim_id) // S2C
			: PlayerEncodedPacket("ba", _id), anim_id(_anim_id) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerEncodedPacket::Append(r);
			AppendPartial(r, anim_id);
			return r;
		}
	protected:
		int anim_id;
	};
	class ShowPlayerBattleAnimDecodedPacket : public PlayerDecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "ba" };
		ShowPlayerBattleAnimDecodedPacket(const ParameterList& v)
			: PlayerDecodedPacket(v.at(0)), anim_id(Decode<int>(v.at(1))) {}
		const int anim_id;
	};

	// ->> unused code

	/**
	 * Sync Switch
	 */

	class SyncSwitchEncodedPacket : public EncodedPacket {
	public:
		SyncSwitchEncodedPacket(int _switch_id, int _value_bin) : EncodedPacket("ss"),
			switch_id(_switch_id), value_bin(_value_bin) {}
		std::string ToBytes() const override { return Build(switch_id, value_bin); }
	protected:
		int switch_id;
		int value_bin;
	};
	class SyncSwitchDecodedPacket : public DecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "ss" };
		SyncSwitchDecodedPacket(const ParameterList& v)
			: switch_id(Decode<int>(v.at(0))), sync_type(Decode<int>(v.at(1))) {}
		const int switch_id;
		const int sync_type;
	};

	/**
	 * Sync Variable
	 */

	class SyncVariableEncodedPacket : public EncodedPacket {
	public:
		SyncVariableEncodedPacket(int _var_id, int _value) : EncodedPacket("sv"),
			var_id(_var_id), value(_value) {}
		std::string ToBytes() const override { return Build(var_id, value); }
	protected:
		int var_id;
		int value;
	};
	class SyncVariableDecodedPacket : public DecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "sv" };
		SyncVariableDecodedPacket(const ParameterList& v)
			: var_id(Decode<int>(v.at(0))), sync_type(Decode<int>(v.at(1))) {}
		const int var_id;
		const int sync_type;
	};

	/**
	 * Sync Event
	 */

	class SyncEventEncodedPacket : public EncodedPacket {
	public:
		SyncEventEncodedPacket(int _event_id, int _action_bin) : EncodedPacket("sev"),
			event_id(_event_id), action_bin(_action_bin) {}
		std::string ToBytes() const override { return Build(event_id, action_bin); }
	protected:
		int event_id;
		int action_bin;
	};
	class SyncEventDecodedPacket : public DecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "sev" };
		SyncEventDecodedPacket(const ParameterList& v)
			: event_id(Decode<int>(v.at(0))), trigger_type(Decode<int>(v.at(1))) {}
		const int event_id;
		const int trigger_type;
	};

	/**
	 * Sync Picture
	 */

	class SyncPictureDecodedPacket : public DecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "sp" };
		SyncPictureDecodedPacket(const ParameterList& v)
			: picture_name(v.at(0)) {}
		std::string picture_name;
	};

	/**
	 * Sync NameList
	 */

	class NameListSyncDecodedPacket : public DecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "pns" };
		NameListSyncDecodedPacket(const ParameterList& v) {
			auto it = v.begin();
			type = Decode<int>(*it);
			++it;
			names.assign(it, v.end());
		}
		int type;
		std::vector<std::string> names;
	};

	/**
	 * Sync Battle Animation Id List
	 */

	class BattleAnimIdListSyncDecodedPacket : public DecodedPacket {
	public:
		constexpr static std::string_view packet_type{ "bas" };
		BattleAnimIdListSyncDecodedPacket(const ParameterList& v) {
			std::transform(v.begin(), v.end(), std::back_inserter(ids),
				[&](std::string_view s) {
					return Decode<int>(s);
				});
		}
		std::vector<int> ids;
	};

	// <<-
}

#endif
