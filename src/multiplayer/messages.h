#ifndef EP_MESSAGES_H
#define EP_MESSAGES_H

#include "connection.h"
#include "packet.h"
#include <memory>
#include <map>
#include <lcf/rpg/sound.h>
#include "../game_pictures.h"

namespace Messages {
	enum VisibilityType : int {
		CV_NULL = 0,
		CV_LOCAL = 1,
		CV_GLOBAL = 2
	};

	static const std::map<VisibilityType, std::string> VisibilityNames = {
		{ CV_LOCAL, "LOCAL" },
		{ CV_GLOBAL, "GLOBAL" }
	};

	static const std::map<std::string, VisibilityType> VisibilityValues = {
		{ "LOCAL", CV_LOCAL },
		{ "GLOBAL", CV_GLOBAL }
	};

	using Packet = Multiplayer::Packet;
	using ParameterList = Multiplayer::Connection::ParameterList;

	/**
	 * Heartbeat
	 */

	class HeartbeatPacket : public Packet {
	public:
		constexpr static std::string_view packet_name{ "hb" };
		HeartbeatPacket() : Packet(packet_name) {}
		HeartbeatPacket(const ParameterList& v) : Packet(packet_name) {}
	};

	/**
	 * Room
	 */

	class RoomPacket : public Packet {
	public:
		constexpr static std::string_view packet_name{ "room" };
		RoomPacket() {}
		RoomPacket(int _room_id) : Packet(packet_name), room_id(_room_id) {}
		std::string ToBytes() const override { return Build(room_id); }
		RoomPacket(const ParameterList& v)
			: Packet(packet_name), room_id(Decode<int>(v.at(0))) {}
		int room_id;
	};

	/**
	 * Base Class: Player
	 */

	class PlayerPacket : public Packet {
	public:
		PlayerPacket(std::string_view _packet_name) // C2S
			: Packet(std::move(_packet_name)) {}
		PlayerPacket(std::string_view _packet_name, int _id) // S2C
			: Packet(std::move(_packet_name)), id(_id) {}
		void Append(std::string& s) const { AppendPartial(s, id); }
		PlayerPacket(std::string_view _packet_name, std::string_view _id)
			: Packet(std::move(_packet_name)), id(Decode<int>(_id)) {}
		int id{0};
	};

	/**
	 * Join
	 */

	class JoinPacket : public PlayerPacket {
	public:
		constexpr static std::string_view packet_name{ "j" };
		JoinPacket() : PlayerPacket(packet_name) {}
		JoinPacket(int _id) : PlayerPacket(packet_name, _id) {} // S2C
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerPacket::Append(r);
			return r;
		}
		JoinPacket(const ParameterList& v)
			: PlayerPacket(packet_name, v.at(0)) {}
	};

	/**
	 * Leave
	 */

	class LeavePacket : public PlayerPacket {
	public:
		constexpr static std::string_view packet_name{ "l" };
		LeavePacket() : PlayerPacket(packet_name) {}
		LeavePacket(int _id) : PlayerPacket(packet_name, _id) {} // S2C
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerPacket::Append(r);
			return r;
		}
		LeavePacket(const ParameterList& v)
			: PlayerPacket(packet_name, v.at(0)) {}
	};

	/**
	 * Name
	 */

	class NamePacket : public PlayerPacket {
	public:
		constexpr static std::string_view packet_name{ "name" };
		NamePacket() : PlayerPacket(packet_name) {}
		NamePacket(std::string _name) // C2S
			: PlayerPacket(packet_name), name(std::move(_name)) {}
		NamePacket(int _id, std::string _name) // S2C
			: PlayerPacket(packet_name, _id), name(std::move(_name)) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerPacket::Append(r);
			AppendPartial(r, name);
			return r;
		}
		NamePacket(const ParameterList& v)
			: PlayerPacket(packet_name, v.at(0)),
			name(v.at(1)) {}
		std::string name;
	};

	/**
	 * Chat
	 */

	class ChatPacket : public PlayerPacket {
	public:
		constexpr static std::string_view packet_name{ "say" };
		ChatPacket() : PlayerPacket(packet_name) {}
		ChatPacket(int _v, std::string _m) // C2S
			: PlayerPacket(packet_name), visibility(_v), message(std::move(_m)) {}
		ChatPacket(int _id, int _t, int _v, int _r, std::string _n, std::string _m) // S2C
			: PlayerPacket(packet_name, _id), type(_t), visibility(_v),
			room_id(_r), name(std::move(_n)), message(std::move(_m)) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerPacket::Append(r);
			AppendPartial(r, type, visibility, room_id, name, message, sys_name);
			return r;
		};
		ChatPacket(const ParameterList& v)
			: PlayerPacket(packet_name, v.at(0)),
			type(Decode<int>(v.at(1))), visibility(Decode<int>(v.at(2))),
			room_id(Decode<int>(v.at(3))),
			name(v.at(4)), message(v.at(5)), sys_name(v.at(6)) {}
		int type; // 0 = info, 1 = chat
		int visibility;
		int room_id;
		std::string name;
		std::string message;
		std::string sys_name;
	};

	/**
	 * Move
	 */

	class MovePacket : public PlayerPacket {
	public:
		constexpr static std::string_view packet_name{ "m" };
		MovePacket() : PlayerPacket(packet_name) {}
		MovePacket(int _x, int _y)
			: PlayerPacket(packet_name), x(_x), y(_y) {}
		MovePacket(int _id, int _x, int _y)
			: PlayerPacket(packet_name, _id), x(_x), y(_y) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerPacket::Append(r);
			AppendPartial(r, x, y);
			return r;
		};
		MovePacket(const ParameterList& v)
			: PlayerPacket(packet_name, v.at(0)),
			x(Decode<int>(v.at(1))), y(Decode<int>(v.at(2))) {}
		int x, y;
	};

	/**
	 * Teleport
	 */

	class TeleportPacket : public Packet {
	public:
		constexpr static std::string_view packet_name{ "tp" };
		TeleportPacket() : Packet(packet_name) {}
		TeleportPacket(int _x, int _y) : Packet(packet_name), x(_x), y(_y) {}
		std::string ToBytes() const override { return Build(x, y); }
		TeleportPacket(const ParameterList& v)
			: Packet(packet_name), x(Decode<int>(v.at(0))), y(Decode<int>(v.at(1))) {}
		int x, y;
	};

	/**
	 * Jump
	 */

	class JumpPacket : public PlayerPacket {
	public:
		constexpr static std::string_view packet_name{ "jmp" };
		JumpPacket() : PlayerPacket(packet_name) {}
		JumpPacket(int _x, int _y) // C2S
			: PlayerPacket(packet_name), x(_x), y(_y) {}
		JumpPacket(int _id, int _x, int _y) // S2C
			: PlayerPacket(packet_name,  _id), x(_x), y(_y) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerPacket::Append(r);
			AppendPartial(r, x, y);
			return r;
		};
		JumpPacket(const ParameterList& v)
			: PlayerPacket(packet_name, v.at(0)),
			x(Decode<int>(v.at(1))), y(Decode<int>(v.at(2))) {}
		int x, y;
	};

	/**
	 * Facing
	 */

	class FacingPacket : public PlayerPacket {
	public:
		constexpr static std::string_view packet_name{ "f" };
		FacingPacket() : PlayerPacket(packet_name) {}
		FacingPacket(int _facing) // C2S
			: PlayerPacket(packet_name), facing(_facing) {}
		FacingPacket(int _id, int _facing) // S2C
			: PlayerPacket(packet_name, _id), facing(_facing) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerPacket::Append(r);
			AppendPartial(r, facing);
			return r;
		};
		FacingPacket(const ParameterList& v)
			: PlayerPacket(packet_name, v.at(0)), facing(Decode<int>(v.at(1))) {}
		int facing{0};
	};

	/**
	 * Speed
	 */

	class SpeedPacket : public PlayerPacket {
	public:
		constexpr static std::string_view packet_name{ "spd" };
		SpeedPacket() : PlayerPacket(packet_name) {}
		SpeedPacket(int _speed) // C2S
			: PlayerPacket(packet_name), speed(_speed) {}
		SpeedPacket(int _id, int _speed) // S2C
			: PlayerPacket(packet_name, _id), speed(_speed) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerPacket::Append(r);
			AppendPartial(r, speed);
			return r;
		};
		SpeedPacket(const ParameterList& v)
			: PlayerPacket(packet_name, v.at(0)), speed(Decode<int>(v.at(1))) {}
		int speed{0};
	};

	/**
	 * Sprite
	 */

	class SpritePacket : public PlayerPacket {
	public:
		constexpr static std::string_view packet_name{ "spr" };
		SpritePacket() : PlayerPacket(packet_name) {}
		SpritePacket(std::string _n, int _i) // C2S
			: PlayerPacket(packet_name), name(_n), index(_i) {}
		SpritePacket(int _id, std::string _n, int _i) // S2C
			: PlayerPacket(packet_name, _id), name(_n), index(_i) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerPacket::Append(r);
			AppendPartial(r, name, index);
			return r;
		};
		SpritePacket(const ParameterList& v)
			: PlayerPacket(packet_name, v.at(0)),
			name(v.at(1)), index(Decode<int>(v.at(2))) {}
		std::string name;
		int index{-1};
	};

	/**
	 * Base Class: Flash
	 */

	class FlashPacket : public PlayerPacket {
	public:
		constexpr static std::string_view packet_name{ "fl" };
		FlashPacket() : PlayerPacket(packet_name) {}
		FlashPacket(int _r, int _g, int _b, int _p, int _f) // C2S
			: PlayerPacket(packet_name), r(_r), g(_g), b(_b), p(_p), f(_f) {}
		FlashPacket(int _id, int _r, int _g, int _b, int _p, int _f) // S2C
			: PlayerPacket(packet_name, _id), r(_r), g(_g), b(_b), p(_p), f(_f) {}
		// custom packet_name
		FlashPacket(std::string_view _packet_name) : PlayerPacket(_packet_name) {}
		FlashPacket(std::string_view _packet_name, int _r, int _g, int _b, int _p, int _f) // C2S
			: PlayerPacket(std::move(_packet_name)), r(_r), g(_g), b(_b), p(_p), f(_f) {}
		FlashPacket(std::string_view _packet_name, int _id, int _r, int _g, int _b, int _p, int _f) // S2C
			: PlayerPacket(std::move(_packet_name), _id), r(_r), g(_g), b(_b), p(_p), f(_f) {}
		std::string ToBytes() const override {
			std::string res {GetName()};
			PlayerPacket::Append(res);
			AppendPartial(res, r, g, b, p, f);
			return res;
		};
		FlashPacket(const ParameterList& v)
			: PlayerPacket(packet_name, v.at(0)),
			r(Decode<int>(v.at(1))), g(Decode<int>(v.at(2))), b(Decode<int>(v.at(3))),
			p(Decode<int>(v.at(4))), f(Decode<int>(v.at(5))) {}
		// custom packet_name
		FlashPacket(std::string_view _packet_name, const ParameterList& v)
			: PlayerPacket(std::move(_packet_name), v.at(0)),
			r(Decode<int>(v.at(1))), g(Decode<int>(v.at(2))), b(Decode<int>(v.at(3))),
			p(Decode<int>(v.at(4))), f(Decode<int>(v.at(5))) {}
		int r, g, b, p, f;
	};

	/**
	 * Repeating Flash
	 */

	class RepeatingFlashPacket : public FlashPacket {
	public:
		constexpr static std::string_view packet_name{ "rfl" };
		RepeatingFlashPacket() : FlashPacket(packet_name) {}
		RepeatingFlashPacket(int _r, int _g, int _b, int _p, int _f) // C2S
			: FlashPacket(packet_name, _r, _g, _b, _p, _f) {}
		RepeatingFlashPacket(int _id, int _r, int _g, int _b, int _p, int _f) // S2C
			: FlashPacket(packet_name, _id, _r, _g, _b, _p, _f) {}
		RepeatingFlashPacket(const ParameterList& v) : FlashPacket(packet_name, v) {}
	};

	/**
	 * Remove Repeating Flash
	 */

	class RemoveRepeatingFlashPacket : public PlayerPacket {
	public:
		constexpr static std::string_view packet_name{ "rrfl" };
		RemoveRepeatingFlashPacket() : PlayerPacket(packet_name) {} // C2S
		RemoveRepeatingFlashPacket(int _id) : PlayerPacket(packet_name, _id) {} // S2C
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerPacket::Append(r);
			return r;
		};
		RemoveRepeatingFlashPacket(const ParameterList& v)
			: PlayerPacket(packet_name, v.at(0)) {}
	};

	/**
	 * Hidden
	 */

	class HiddenPacket : public PlayerPacket {
	public:
		constexpr static std::string_view packet_name{ "h" };
		HiddenPacket() : PlayerPacket(packet_name) {}
		HiddenPacket(int _hidden_bin) // C2S
			: PlayerPacket(packet_name), hidden_bin(_hidden_bin) {}
		HiddenPacket(int _id, int _hidden_bin) // S2C
			: PlayerPacket(packet_name, _id), hidden_bin(_hidden_bin) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerPacket::Append(r);
			AppendPartial(r, hidden_bin);
			return r;
		};
		HiddenPacket(const ParameterList& v)
			: PlayerPacket(packet_name, v.at(0)),
			hidden_bin(Decode<int>(v.at(1))) {}
		int hidden_bin{0};
	};

	/**
	 * System Graphic
	 */

	class SystemPacket : public PlayerPacket {
	public:
		constexpr static std::string_view packet_name{ "sys" };
		SystemPacket() : PlayerPacket(packet_name) {}
		SystemPacket(std::string _name) // C2S
			: PlayerPacket(packet_name), name(std::move(_name)) {}
		SystemPacket(int _id, std::string _name) // S2C
			: PlayerPacket(packet_name, _id), name(std::move(_name)) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerPacket::Append(r);
			AppendPartial(r, name);
			return r;
		};
		SystemPacket(const ParameterList& v)
			: PlayerPacket(packet_name, v.at(0)), name(v.at(1)) {}
		std::string name{""};
	};

	/**
	 * Sound Effect
	 */

	class SEPacket : public PlayerPacket {
	public:
		constexpr static std::string_view packet_name{ "se" };
		SEPacket() : PlayerPacket(packet_name) {}
		SEPacket(lcf::rpg::Sound _d) // C2S
			: PlayerPacket(packet_name), snd(std::move(_d)) {}
		SEPacket(int _id, lcf::rpg::Sound _d) // S2C
			: PlayerPacket(packet_name, _id), snd(std::move(_d)) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerPacket::Append(r);
			AppendPartial(r, snd.name, snd.volume, snd.tempo, snd.balance);
			return r;
		};
		static lcf::rpg::Sound BuildSound(const ParameterList& v) {
			lcf::rpg::Sound s;
			s.name = v.at(1);
			s.volume = Decode<int>(v.at(2));
			s.tempo = Decode<int>(v.at(3));
			s.balance = Decode<int>(v.at(4));
			return s;
		}
		SEPacket(const ParameterList& v)
			: PlayerPacket(packet_name, v.at(0)), snd(BuildSound(v)) {}
		lcf::rpg::Sound snd;
	};

	/**
	 * Base Class: Picture
	 */

	class PicturePacket : public PlayerPacket {
	public:
		PicturePacket(std::string_view _packet_name, int _pic_id, Game_Pictures::Params& _p, // C2S
				int _mx, int _my, int _panx, int _pany)
			: PlayerPacket(std::move(_packet_name)), pic_id(_pic_id), params(_p),
			map_x(_mx), map_y(_my), pan_x(_panx), pan_y(_pany) {}
		PicturePacket(std::string_view _packet_name, int _id, int _pic_id, Game_Pictures::Params& _p, // S2C
				int _mx, int _my, int _panx, int _pany)
			: PlayerPacket(std::move(_packet_name), _id), pic_id(_pic_id), params(_p),
			map_x(_mx), map_y(_my), pan_x(_panx), pan_y(_pany) {}
		void Append(std::string& s) const {
			PlayerPacket::Append(s);
			AppendPartial(s, pic_id, params.position_x, params.position_y,
					map_x, map_y, pan_x, pan_y,
					params.magnify, params.top_trans, params.bottom_trans,
					params.red, params.green, params.blue, params.saturation,
					params.effect_mode, params.effect_power);
		}
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
		PicturePacket(std::string_view _packet_name, Game_Pictures::Params& _params, const ParameterList& v)
			: PlayerPacket(std::move(_packet_name), v.at(0)), params(_params),
			pic_id(Decode<int>(v.at(1))),
			map_x(Decode<int>(v.at(4))), map_y(Decode<int>(v.at(5))),
			pan_x(Decode<int>(v.at(6))), pan_y(Decode<int>(v.at(7))) {}
		// skip Game_Pictures::Params&
		PicturePacket& operator=(const PicturePacket& o) {
			id = o.id;
			pic_id = o.pic_id;
			map_x = o.map_x; map_y = o.map_y;
			pan_x = o.pan_x; pan_y = o.pan_y;
			return *this;
		}
		int pic_id;
		Game_Pictures::Params& params;
		int map_x, map_y;
		int pan_x, pan_y;
	};

	/**
	 * Show Picture
	 */

	class ShowPicturePacket : public PicturePacket {
	public:
		constexpr static std::string_view packet_name{ "ap" };
		ShowPicturePacket() : PicturePacket(packet_name, 0, params, 0, 0, 0, 0) {}
		ShowPicturePacket(int _pid, Game_Pictures::ShowParams _p, // C2S
				int _mx, int _my, int _px, int _py)
			: PicturePacket(packet_name, _pid, params, _mx, _my, _px, _py), params(std::move(_p)) {}
		ShowPicturePacket(int _id, int _pid, Game_Pictures::ShowParams _p, // S2C
				int _mx, int _my, int _px, int _py)
			: PicturePacket(packet_name, _id, _pid, params, _mx, _my, _px, _py), params(std::move(_p)) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PicturePacket::Append(r);
			AppendPartial(r, params.name, params.use_transparent_color, params.fixed_to_map);
			return r;
		}
		Game_Pictures::ShowParams BuildParams(const ParameterList& v) const {
			Game_Pictures::ShowParams p;
			PicturePacket::BuildParams(p, v);
			p.name = v.at(17);
			p.use_transparent_color = Decode<bool>(v.at(18));
			p.fixed_to_map = Decode<bool>(v.at(19));
			return p;
		}
		ShowPicturePacket(const ParameterList& v)
			: PicturePacket(packet_name, params, v), params(BuildParams(v)) {}
		Game_Pictures::ShowParams params;
	};

	/**
	 * Move Picture
	 */

	class MovePicturePacket : public PicturePacket {
	public:
		constexpr static std::string_view packet_name{ "mp" };
		MovePicturePacket() : PicturePacket(packet_name, 0, params, 0, 0, 0, 0) {}
		MovePicturePacket(int _pid, Game_Pictures::MoveParams _p, // C2S
				int _mx, int _my, int _px, int _py)
			: PicturePacket(packet_name, _pid, params, _mx, _my, _px, _py), params(std::move(_p)) {}
		MovePicturePacket(int _id, int _pid, Game_Pictures::MoveParams _p, // S2C
				int _mx, int _my, int _px, int _py)
			: PicturePacket(packet_name, _id, _pid, params, _mx, _my, _px, _py), params(std::move(_p)) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PicturePacket::Append(r);
			AppendPartial(r, params.duration);
			return r;
		}
		Game_Pictures::MoveParams BuildParams(const ParameterList& v) const {
			Game_Pictures::MoveParams p;
			PicturePacket::BuildParams(p, v);
			p.duration = Decode<int>(v.at(17));
			return p;
		}
		MovePicturePacket(const ParameterList& v)
			: PicturePacket(packet_name, params, v), params(BuildParams(v)) {}
		Game_Pictures::MoveParams params;
	};

	/**
	 * Erase Picture
	 */

	class ErasePicturePacket : public PlayerPacket {
	public:
		constexpr static std::string_view packet_name{ "rp" };
		ErasePicturePacket() : PlayerPacket(packet_name) {}
		ErasePicturePacket(int _pid) : PlayerPacket(packet_name), pic_id(_pid) {} // C2S
		ErasePicturePacket(int _id, int _pid) : PlayerPacket(packet_name, _id), pic_id(_pid) {} // S2C
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerPacket::Append(r);
			AppendPartial(r, pic_id);
			return r;
		}
		ErasePicturePacket(const ParameterList& v)
			: PlayerPacket(packet_name, v.at(0)), pic_id(Decode<int>(v.at(1))) {}
		int pic_id;
	};

	/**
	 * Show Player Battle Animation
	 */

	class ShowPlayerBattleAnimPacket : public PlayerPacket {
	public:
		constexpr static std::string_view packet_name{ "ba" };
		ShowPlayerBattleAnimPacket() : PlayerPacket(packet_name) {}
		ShowPlayerBattleAnimPacket(int _anim_id) // C2S
			: PlayerPacket(packet_name), anim_id(_anim_id) {}
		ShowPlayerBattleAnimPacket(int _id, int _anim_id) // S2C
			: PlayerPacket(packet_name, _id), anim_id(_anim_id) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PlayerPacket::Append(r);
			AppendPartial(r, anim_id);
			return r;
		}
		ShowPlayerBattleAnimPacket(const ParameterList& v)
			: PlayerPacket(packet_name, v.at(0)), anim_id(Decode<int>(v.at(1))) {}
		int anim_id;
	};

	/**
	 * Allowed Picture Names
	 */

	class PictureNameListSyncPacket : public Packet {
	public:
		constexpr static std::string_view packet_name{ "pns" };
		PictureNameListSyncPacket() : Packet(packet_name) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			AppendPartial(r, type);
			for (const std::string& n : names)
				AppendPartial(r, n);
			return r;
		}
		PictureNameListSyncPacket(const ParameterList& v) : Packet(packet_name) {
			auto it = v.begin();
			type = Decode<int>(*it);
			++it;
			names.assign(it, v.end());
		}
		int type;
		std::vector<std::string> names;
	};

	/**
	 * Allowed Battle Animation Ids
	 */

	class BattleAnimIdListSyncPacket : public Packet {
	public:
		constexpr static std::string_view packet_name{ "bas" };
		BattleAnimIdListSyncPacket() : Packet(packet_name) {}
		BattleAnimIdListSyncPacket(const ParameterList& v) : Packet(packet_name) {
			std::transform(v.begin(), v.end(), std::back_inserter(ids),
				[&](std::string_view s) {
					return Decode<int>(s);
				});
		}
		std::vector<int> ids;
	};

	// ->> unused code

	/**
	 * Sync Switch
	 */

	class SyncSwitchPacket : public Packet {
	public:
		constexpr static std::string_view packet_name{ "ss" };
		SyncSwitchPacket() : Packet(packet_name) {}
		SyncSwitchPacket(int _switch_id, int _sync_type)
			: Packet(packet_name), switch_id(_switch_id), sync_type(_sync_type) {}
		std::string ToBytes() const override { return Build(switch_id, sync_type); }
		SyncSwitchPacket(const ParameterList& v)
			: Packet(packet_name), switch_id(Decode<int>(v.at(0))), sync_type(Decode<int>(v.at(1))) {}
		int switch_id;
		int sync_type;
	};

	/**
	 * Sync Variable
	 */

	class SyncVariablePacket : public Packet {
	public:
		constexpr static std::string_view packet_name{ "sv" };
		SyncVariablePacket() : Packet(packet_name) {}
		SyncVariablePacket(int _var_id, int _value) : Packet(packet_name),
			var_id(_var_id), sync_type(_value) {}
		std::string ToBytes() const override { return Build(var_id, sync_type); }
		SyncVariablePacket(const ParameterList& v)
			: Packet(packet_name), var_id(Decode<int>(v.at(0))), sync_type(Decode<int>(v.at(1))) {}
		int var_id;
		int sync_type;
	};

	/**
	 * Sync Event
	 */

	class SyncEventPacket : public Packet {
	public:
		constexpr static std::string_view packet_name{ "sev" };
		SyncEventPacket() : Packet(packet_name) {}
		SyncEventPacket(int _event_id, int _trigger_type) : Packet(packet_name),
			event_id(_event_id), trigger_type(_trigger_type) {}
		std::string ToBytes() const override { return Build(event_id, trigger_type); }
		SyncEventPacket(const ParameterList& v)
			: Packet(packet_name),
			event_id(Decode<int>(v.at(0))), trigger_type(Decode<int>(v.at(1))) {}
		int event_id;
		int trigger_type;
	};

	/**
	 * Sync Picture
	 */

	class SyncPicturePacket : public Packet {
	public:
		constexpr static std::string_view packet_name{ "sp" };
		SyncPicturePacket() : Packet(packet_name) {}
		SyncPicturePacket(const ParameterList& v)
			: Packet(packet_name), picture_name(v.at(0)) {}
		std::string picture_name;
	};

	// <<-
}

#endif
