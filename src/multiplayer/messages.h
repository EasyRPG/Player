#ifndef EP_MESSAGES_H
#define EP_MESSAGES_H

#include "connection.h"
#include "packet.h"
#include <memory>
#include <lcf/rpg/sound.h>
#include "../game_pictures.h"

namespace Messages {
namespace S2C {
	using S2CPacket = Multiplayer::S2CPacket;
	using PL = Multiplayer::Connection::ParameterList;

	class DummyPacket : public S2CPacket {
	public:
		DummyPacket(const PL&) {}
	};

	class SyncPlayerDataPacket : public S2CPacket {
	public:
		SyncPlayerDataPacket(const PL& v)
			: host_id(Decode<int>(v.at(0))),
			key(v.at(1)),
			uuid(v.at(2)),
			rank(Decode<int>(v.at(3))),
			account_bin(Decode<int>(v.at(4))),
			badge(v.at(5)),
			medals{
				Decode<int>(v.at(6)),
				Decode<int>(v.at(7)),
				Decode<int>(v.at(8)),
				Decode<int>(v.at(9)),
				Decode<int>(v.at(10))
			} {}

		const int host_id;
		const std::string key;
		const std::string uuid;
		const int rank;
		const int account_bin;
		const std::string badge;
		const int medals[5];
	};

	class RoomInfoPacket : public S2CPacket {
		public:
			RoomInfoPacket(const PL& v)
			: room_id(Decode<int>(v.at(0))) {}

		const int room_id;
	};

	class PlayerPacket : public S2CPacket {
	public:
		PlayerPacket(std::string_view _id) : id(Decode<int>(_id)) {}
		bool IsCurrent(int host_id) const { return id == host_id; }
		const int id;
	};

	class ConnectPacket : public PlayerPacket {
	public:
		ConnectPacket(const PL& v)
			: PlayerPacket(v.at(0)),
			uuid(v.at(1)),
			rank(Decode<int>(v.at(2))),
			account_bin(Decode<int>(v.at(3))),
			badge(v.at(4)),
			medals{
				Decode<int>(v.at(5)),
				Decode<int>(v.at(6)),
				Decode<int>(v.at(7)),
				Decode<int>(v.at(8)),
				Decode<int>(v.at(9))
			} {}
		const std::string uuid;
		const int rank;
		const int account_bin;
		const std::string badge;
		int medals[5];
	};

	class DisconnectPacket : public PlayerPacket {
	public:
		DisconnectPacket(const PL& v)
			: PlayerPacket(v.at(0)) {}
	};

	class ChatPacket : public PlayerPacket {
	public:
		ChatPacket(const PL& v)
			: PlayerPacket(v.at(0)),
			msg(v.at(1)) {}
		const std::string msg;
	};

	class MovePacket : public PlayerPacket {
	public:
		MovePacket(const PL& v)
			: PlayerPacket(v.at(0)),
			x(Decode<int>(v.at(1))),
			y(Decode<int>(v.at(2))) {}
		const int x, y;
	};

	class JumpPacket : public PlayerPacket {
	public:
		JumpPacket(const PL& v)
			: PlayerPacket(v.at(0)),
			x(Decode<int>(v.at(1))),
			y(Decode<int>(v.at(2))) {}
		const int x, y;
	};

	class FacingPacket : public PlayerPacket {
	public:
		FacingPacket(const PL& v)
			: PlayerPacket(v.at(0)),
			facing(Decode<int>(v.at(1))) {}
		const int facing;
	};

	class SpeedPacket : public PlayerPacket {
	public:
		SpeedPacket(const PL& v)
			: PlayerPacket(v.at(0)),
			speed(Decode<int>(v.at(1))) {}
		const int speed;
	};

	class SpritePacket : public PlayerPacket {
	public:
		SpritePacket(const PL& v)
			: PlayerPacket(v.at(0)),
			name(v.at(1)),
			index(Decode<int>(v.at(2))) {}
		const std::string name;
		const int index;
	};

	class FlashPacket : public PlayerPacket {
	public:
		FlashPacket(const PL& v)
			: PlayerPacket(v.at(0)),
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

	class RepeatingFlashPacket : public FlashPacket {
	public:
		RepeatingFlashPacket(const PL& v)
			: FlashPacket(v) {}
	};

	class RemoveRepeatingFlashPacket : public PlayerPacket {
	public:
		RemoveRepeatingFlashPacket(const PL& v)
			: PlayerPacket(v.at(0)) {}
	};

	class HiddenPacket : public PlayerPacket {
	public:
		HiddenPacket(const PL& v)
			: PlayerPacket(v.at(0)),
			hidden_bin(Decode<int>(v.at(1))) {}
		const int hidden_bin;
	};

	class SystemPacket : public PlayerPacket {
	public:
		SystemPacket(const PL& v)
			: PlayerPacket(v.at(0)),
			name(v.at(1)) {}
		const std::string name;
	};

	class SEPacket : public PlayerPacket {
	public:
		static lcf::rpg::Sound BuildSound(const PL& v) {
			lcf::rpg::Sound s;
			s.name = v.at(1);
			s.volume = Decode<int>(v.at(2));
			s.tempo = Decode<int>(v.at(3));
			s.balance = Decode<int>(v.at(4));
			return s;
		}
		SEPacket(const PL& v)
			: PlayerPacket(v.at(0)),
			snd(BuildSound(v)) {}
		const lcf::rpg::Sound snd;
	};

	class PicturePacket : public PlayerPacket {
	public:
		static void BuildParams(Game_Pictures::Params& p, const PL& v) {
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
		PicturePacket(Game_Pictures::Params& _params, const PL& v)
			: PlayerPacket(v.at(0)), params(_params),
			pic_id(Decode<int>(v.at(1))),
			map_x(Decode<int>(v.at(4))), map_y(Decode<int>(v.at(5))),
			pan_x(Decode<int>(v.at(6))), pan_y(Decode<int>(v.at(7))) {}
		Game_Pictures::Params& params;
		const int pic_id;
		int map_x, map_y;
		int pan_x, pan_y;
	};

	class ShowPicturePacket : public PicturePacket {
	public:
		Game_Pictures::ShowParams BuildParams(const PL& v) const {
			Game_Pictures::ShowParams p;
			PicturePacket::BuildParams(p, v);
			p.name = v.at(17);
			p.use_transparent_color = Decode<bool>(v.at(18));
			p.fixed_to_map = Decode<bool>(v.at(19));
			return p;
		}
		ShowPicturePacket(const PL& v)
			: PicturePacket(params, v),
			params(BuildParams(v)) {}
		Game_Pictures::ShowParams params;
	};

	class MovePicturePacket : public PicturePacket {
	public:
		Game_Pictures::MoveParams BuildParams(const PL& v) const {
			Game_Pictures::MoveParams p;
			PicturePacket::BuildParams(p, v);
			p.duration = Decode<int>(v.at(17));
			return p;
		}
		MovePicturePacket(const PL& v)
			: PicturePacket(params, v),
			params(BuildParams(v)) {}
		Game_Pictures::MoveParams params;
	};

	class ErasePicturePacket : public PlayerPacket {
	public:
		ErasePicturePacket(const PL& v)
			: PlayerPacket(v.at(0)), pic_id(Decode<int>(v.at(1))) {}
		const int pic_id;
	};

	class ShowPlayerBattleAnimPacket : public PlayerPacket {
	public:
		ShowPlayerBattleAnimPacket(const PL& v)
			: PlayerPacket(v.at(0)), anim_id(Decode<int>(v.at(1))) {}
		const int anim_id;
	};

	class NamePacket : public PlayerPacket {
	public:
		NamePacket(const PL& v)
			: PlayerPacket(v.at(0)),
			name(v.at(1)) {}
		const std::string name;
	};

	class SyncSwitchPacket : public S2CPacket {
	public:
		SyncSwitchPacket(const PL& v)
			: switch_id(Decode<int>(v.at(0))), sync_type(Decode<int>(v.at(1))) {}

		const int switch_id;
		const int sync_type;
	};

	class SyncVariablePacket : public S2CPacket {
	public:
		SyncVariablePacket(const PL& v)
			: var_id(Decode<int>(v.at(0))), sync_type(Decode<int>(v.at(1))) {}

		const int var_id;
		const int sync_type;
	};

	class SyncEventPacket : public S2CPacket {
	public:
		SyncEventPacket(const PL& v)
			: event_id(Decode<int>(v.at(0))), trigger_type(Decode<int>(v.at(1))) {}

		const int event_id;
		const int trigger_type;
	};

	class SyncPicturePacket : public S2CPacket {
	public:
		SyncPicturePacket(const PL& v)
			: picture_name(v.at(0)) {}

		std::string picture_name;
	};

	class NameListSyncPacket : public S2CPacket {
	public:
		NameListSyncPacket(const PL& v) {
			auto it = v.begin();
			type = Decode<int>(*it);
			++it;
			names.assign(it, v.end());
		}
		int type;
		std::vector<std::string> names;
	};

	class BattleAnimIdListSyncPacket : public S2CPacket {
	public:
		BattleAnimIdListSyncPacket(const PL& v) {
			std::transform(v.begin(), v.end(), std::back_inserter(ids),
				[&](std::string_view s) {
					return Decode<int>(s);
				});
		}
		std::vector<int> ids;
	};

	class BadgeUpdatePacket : public S2CPacket {
	public:
		BadgeUpdatePacket(const PL& v) {}
	};
}
namespace C2S {
	using C2SPacket = Multiplayer::C2SPacket;

	class SwitchRoomPacket : public C2SPacket {
	public:
		SwitchRoomPacket(int _id) : C2SPacket("sr"), id(_id) {}
		std::string ToBytes() const override { return Build(id); }
	protected:
		int id;
	};

	class MainPlayerPosPacket : public C2SPacket {
	public:
		MainPlayerPosPacket(int _x, int _y) : C2SPacket("m"),
			x(_x), y(_y) {}
		std::string ToBytes() const override { return Build(x, y); }
	protected:
		int x, y;
	};

	class TeleportPacket : public C2SPacket {
	public:
		TeleportPacket(int _x, int _y) : C2SPacket("tp"),
			x(_x), y(_y) {}
		std::string ToBytes() const override { return Build(x, y); }
	protected:
		int x, y;
	};

	class JumpPacket : public C2SPacket {
	public:
		JumpPacket(int _x, int _y) : C2SPacket("jmp"),
			x(_x), y(_y) {}
		std::string ToBytes() const override { return Build(x, y); }
	protected:
		int x, y;
	};

	class FacingPacket : public C2SPacket {
	public:
		FacingPacket(int _d) : C2SPacket("f"), d(_d) {}
		std::string ToBytes() const override { return Build(d); }
	protected:
		int d;
	};

	class SpeedPacket : public C2SPacket {
	public:
		SpeedPacket(int _spd) : C2SPacket("spd"), spd(_spd) {}
		std::string ToBytes() const override { return Build(spd); }
	protected:
		int spd;
	};

	class SpritePacket : public C2SPacket {
	public:
		SpritePacket(std::string _n, int _i) : C2SPacket("spr"),
			name(_n), index(_i) {}
		std::string ToBytes() const override { return Build(name, index); }
	protected:
		std::string name;
		int index;
	};

	class FlashPacket : public C2SPacket {
	public:
		FlashPacket(int _r, int _g, int _b, int _p, int _f) : C2SPacket("fl"),
			r(_r), g(_g), b(_b), p(_p), f(_f) {}
		std::string ToBytes() const override { return Build(r, g, b, p, f); }
	protected:
		int r;
		int g;
		int b;
		int p;
		int f;
	};

	class RepeatingFlashPacket : public C2SPacket {
	public:
		RepeatingFlashPacket(int _r, int _g, int _b, int _p, int _f) : C2SPacket("rfl"),
			r(_r), g(_g), b(_b), p(_p), f(_f) {}
		std::string ToBytes() const override { return Build(r, g, b, p, f); }
	protected:
		int r;
		int g;
		int b;
		int p;
		int f;
	};

	class RemoveRepeatingFlashPacket : public C2SPacket {
	public:
		RemoveRepeatingFlashPacket() : C2SPacket("rrfl") {}
		std::string ToBytes() const override { return Build(); }
	};

	class HiddenPacket : public C2SPacket {
	public:
		HiddenPacket(int _hidden_bin) : C2SPacket("h"),
			hidden_bin(_hidden_bin) {}
		std::string ToBytes() const override { return Build(hidden_bin); }
	protected:
		int hidden_bin;
	};

	class SEPacket : public C2SPacket {
	public:
		SEPacket(lcf::rpg::Sound _d) : C2SPacket("se"), snd(std::move(_d)) {}
		std::string ToBytes() const override { return Build(snd.name, snd.volume, snd.tempo, snd.balance); }
	protected:
		lcf::rpg::Sound snd;
	};

	class SysNamePacket : public C2SPacket {
	public:
		SysNamePacket(std::string _s) : C2SPacket("sys"), s(std::move(_s)) {}
		std::string ToBytes() const override { return Build(s); }
	protected:
		std::string s;
	};

	class PicturePacket : public C2SPacket {
	public:
		PicturePacket(std::string _name, int _pic_id, Game_Pictures::Params& _p,
				int _mx, int _my,
				int _panx, int _pany)
			: C2SPacket(std::move(_name)), pic_id(_pic_id), p(_p),
		map_x(_mx), map_y(_my),
		pan_x(_panx), pan_y(_pany) {}
		void Append(std::string& s) const {
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

	class ShowPicturePacket : public PicturePacket {
	public:
		ShowPicturePacket(int _pid, Game_Pictures::ShowParams _p,
				int _mx, int _my, int _px, int _py)
			: PicturePacket("ap", _pid, p_show, _mx, _my, _px, _py), p_show(std::move(_p)) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PicturePacket::Append(r);
			AppendPartial(r, p_show.name, p_show.use_transparent_color, p_show.fixed_to_map);
			return r;
		}
	protected:
		Game_Pictures::ShowParams p_show;
	};

	class MovePicturePacket : public PicturePacket {
	public:
		MovePicturePacket(int _pid, Game_Pictures::MoveParams _p,
				int _mx, int _my, int _px, int _py)
			: PicturePacket("mp", _pid, p_move, _mx, _my, _px, _py), p_move(std::move(_p)) {}
		std::string ToBytes() const override {
			std::string r {GetName()};
			PicturePacket::Append(r);
			AppendPartial(r, p_move.duration);
			return r;
		}
	protected:
		Game_Pictures::MoveParams p_move;
	};

	class ErasePicturePacket : public C2SPacket {
	public:
		ErasePicturePacket(int _pid) : C2SPacket("rp"), pic_id(_pid) {}
		std::string ToBytes() const override { return Build(pic_id); }
	protected:
		int pic_id;
	};

	class ShowPlayerBattleAnimPacket : public C2SPacket {
	public:
		ShowPlayerBattleAnimPacket(int _anim_id) : C2SPacket("ba"),
			anim_id(_anim_id) {}
		std::string ToBytes() const override { return Build(anim_id); }
	protected:
		int anim_id;
	};

	class ChatPacket : public C2SPacket {
	public:
		ChatPacket(std::string _msg) : C2SPacket("say"),
			msg(std::move(_msg)) {}
		std::string ToBytes() const override { return Build(msg); }
	protected:
		std::string msg;
	};

	class SyncSwitchPacket : public C2SPacket {
	public:
		SyncSwitchPacket(int _switch_id, int _value_bin) : C2SPacket("ss"),
			switch_id(_switch_id), value_bin(_value_bin) {}
		std::string ToBytes() const override { return Build(switch_id, value_bin); }
	protected:
		int switch_id;
		int value_bin;
	};

	class SyncVariablePacket : public C2SPacket {
	public:
		SyncVariablePacket(int _var_id, int _value) : C2SPacket("sv"),
			var_id(_var_id), value(_value) {}
		std::string ToBytes() const override { return Build(var_id, value); }
	protected:
		int var_id;
		int value;
	};

	class SyncEventPacket : public C2SPacket {
	public:
		SyncEventPacket(int _event_id, int _action_bin) : C2SPacket("sev"),
			event_id(_event_id), action_bin(_action_bin) {}
		std::string ToBytes() const override { return Build(event_id, action_bin); }
	protected:
		int event_id;
		int action_bin;
	};

}
}

#endif
