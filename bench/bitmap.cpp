#include <cmath>
#include <benchmark/benchmark.h>
#include <rect.h>
#include <bitmap.h>
#include <pixel_format.h>
#include <transform.h>

constexpr auto opacity_100 = Opacity::Opaque();
constexpr auto opacity_0 = Opacity(0);
constexpr auto opacity_50 = Opacity(128);
constexpr auto opacity_75_25 = Opacity(192, 64, 1);

constexpr auto opacity = opacity_100;

const auto fmt_rgba = format_R8G8B8A8_a().format();
const auto fmt_bgra = format_B8G8R8A8_a().format();
const auto fmt_argb = format_A8R8G8B8_a().format();
const auto fmt_abgr = format_A8B8G8R8_a().format();

const DynamicFormat formats[] = { fmt_rgba, fmt_bgra, fmt_argb, fmt_abgr };
const auto format = fmt_rgba;

struct BitmapAccess : public Bitmap {
	static pixman_format_code_t find_format(const DynamicFormat& format) {
		return Bitmap::find_format(format);
	}
};

static void BM_FindFormatSingle(benchmark::State& state) {
	for (auto _: state) {
		BitmapAccess::find_format(format);
	}
}

BENCHMARK(BM_FindFormatSingle);

static void BM_FindFormat(benchmark::State& state) {
	for (auto _: state) {
		for (auto& f: formats) {
			BitmapAccess::find_format(f);
		}
	}
}

BENCHMARK(BM_FindFormat);

static void BM_ComputeImageOpacity(benchmark::State& state) {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	auto bm = Bitmap::Create(320, 240);
	for (auto _: state) {
		bm->ComputeImageOpacity();
	}
}

BENCHMARK(BM_ComputeImageOpacity);

static void BM_ComputeImageOpacityChipset(benchmark::State& state) {
	Bitmap::SetFormat(format_R8G8B8A8_a().format());
	const int w = 480;
	const int h = 256;
	const int dx = 16;
	const int dy = 16;
	auto bm = Bitmap::Create(w, h);
	for (auto _: state) {
		for (int y = 0; y < h; y += dy) {
			for (int x = 0; x < w; x += dx) {
				bm->ComputeImageOpacity(Rect{ x, y, dx, dy });
			}
		}
	}
}

BENCHMARK(BM_ComputeImageOpacityChipset);

static void BM_Create(benchmark::State& state) {
	Bitmap::SetFormat(format);
	for (auto _: state) {
		auto bm = Bitmap::Create(320, 240);
		(void)bm;
	}
}

BENCHMARK(BM_Create);

static void BM_Blit(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto src = Bitmap::Create(320, 240);
	auto rect = src->GetRect();
	for (auto _: state) {
		dest->Blit(0, 0, *src, rect, opacity);
	}
}

BENCHMARK(BM_Blit);

static void BM_BlitFast(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto src = Bitmap::Create(320, 240);
	auto rect = src->GetRect();
	for (auto _: state) {
		dest->BlitFast(0, 0, *src, rect, opacity);
	}
}

BENCHMARK(BM_BlitFast);

static void BM_TiledBlit(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto src = Bitmap::Create(16, 16);
	auto rect = src->GetRect();
	for (auto _: state) {
		dest->TiledBlit(rect, *src, rect, opacity);
	}
}

BENCHMARK(BM_TiledBlit);

static void BM_TiledBlitOffset(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto src = Bitmap::Create(64, 64);
	auto rect = src->GetRect();
	for (auto _: state) {
		dest->TiledBlit(32, 32, Rect{32,32,16,16}, *src, rect, opacity);
	}
}

BENCHMARK(BM_TiledBlitOffset);

static void BM_StretchBlit(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto src = Bitmap::Create(20, 20);
	auto rect = src->GetRect();
	for (auto _: state) {
		dest->StretchBlit(*src, rect, opacity);
	}
}

BENCHMARK(BM_StretchBlit);

static void BM_StretchBlitRect(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto dst_rect = dest->GetRect();
	auto src = Bitmap::Create(20, 20);
	auto rect = src->GetRect();
	for (auto _: state) {
		dest->StretchBlit(dst_rect, *src, rect, opacity);
	}
}

BENCHMARK(BM_StretchBlitRect);

static void BM_FlipBlit(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto src = Bitmap::Create(320, 240);
	auto rect = src->GetRect();
	for (auto _: state) {
		dest->FlipBlit(0, 0, *src, rect, true, true, opacity);
	}
}

BENCHMARK(BM_FlipBlit);

static void BM_ZoomOpacityBlit(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto src = Bitmap::Create(60, 60);
	auto rect = src->GetRect();
	for (auto _: state) {
		dest->ZoomOpacityBlit(0, 0, 30, 30, *src, rect, 2.0, 2.0, opacity);
	}
}

BENCHMARK(BM_ZoomOpacityBlit);

static void BM_RotateZoomOpacityBlit(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto src = Bitmap::Create(60, 60);
	auto rect = src->GetRect();
	for (auto _: state) {
		dest->RotateZoomOpacityBlit(0, 0, 30, 30, *src, rect, M_PI, 2.0, 2.0, opacity);
	}
}

BENCHMARK(BM_RotateZoomOpacityBlit);

static void BM_WaverBlit(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto src = Bitmap::Create(320, 240);
	auto rect = src->GetRect();
	auto zoom_x = 2.0;
	auto zoom_y = 2.0;
	int depth = 2;
	double phase = M_PI;
	for (auto _: state) {
		dest->WaverBlit(0, 0, zoom_x, zoom_y, *src, rect, depth, phase, opacity);
	}
}

BENCHMARK(BM_WaverBlit);

static void BM_Fill(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto color = Color(255, 255, 255, 255);
	for (auto _: state) {
		dest->Fill(color);
	}
}

BENCHMARK(BM_Fill);

static void BM_FillRect(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto rect = dest->GetRect();
	auto color = Color(255, 255, 255, 255);
	for (auto _: state) {
		dest->FillRect(rect, color);
	}
}

BENCHMARK(BM_FillRect);

static void BM_Clear(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	for (auto _: state) {
		dest->Clear();
	}
}

BENCHMARK(BM_Clear);

static void BM_ClearRect(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto rect = dest->GetRect();
	for (auto _: state) {
		dest->ClearRect(rect);
	}
}

BENCHMARK(BM_ClearRect);

static void BM_HueChangeBlit(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto src = Bitmap::Create(320, 240);
	auto rect = src->GetRect();
	double hue = 1.0;
	for (auto _: state) {
		dest->HueChangeBlit(0, 0, *src, rect, hue);
	}
}

BENCHMARK(BM_HueChangeBlit);

static void BM_ToneBlit(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto src = Bitmap::Create(320, 240);
	auto rect = src->GetRect();
	auto tone = Tone(255,255,255,128);
	for (auto _: state) {
		dest->ToneBlit(0, 0, *src, rect, tone, opacity);
	}
}

BENCHMARK(BM_ToneBlit);

static void BM_BlendBlit(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto src = Bitmap::Create(320, 240);
	auto rect = src->GetRect();
	auto color = Color(255, 255, 255, 255);
	for (auto _: state) {
		dest->BlendBlit(0, 0, *src, rect, color, opacity);
	}
}

BENCHMARK(BM_BlendBlit);

static void BM_Flip(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	for (auto _: state) {
		dest->Flip(true, true);
	}
}

BENCHMARK(BM_Flip);

static void BM_MaskedBlit(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto dst_rect = dest->GetRect();
	auto src = Bitmap::Create(320, 240);
	auto mask = Bitmap::Create(320, 240);
	for (auto _: state) {
		dest->MaskedBlit(dst_rect, *mask, 0, 0, *src, 0, 0);
	}
}

BENCHMARK(BM_MaskedBlit);

static void BM_MaskedColorBlit(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto dst_rect = dest->GetRect();
	auto mask = Bitmap::Create(320, 240);
	auto color = Color(255, 255, 255, 255);
	for (auto _: state) {
		dest->MaskedBlit(dst_rect, *mask, 0, 0, color);
	}
}

BENCHMARK(BM_MaskedColorBlit);

static void BM_Blit2x(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto dst_rect = dest->GetRect();
	auto src = Bitmap::Create(320, 240);
	auto rect = src->GetRect();
	for (auto _: state) {
		dest->Blit2x(dst_rect, *src, rect);
	}
}

BENCHMARK(BM_Blit2x);

static void BM_TransformRectangle(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto rect = dest->GetRect();
	auto xform = Transform::Rotation(M_PI);
	for (auto _: state) {
		dest->TransformRectangle(xform, rect);
	}
}

BENCHMARK(BM_TransformRectangle);

static void BM_EffectsBlit(benchmark::State& state) {
	Bitmap::SetFormat(format);
	auto dest = Bitmap::Create(320, 240);
	auto src = Bitmap::Create(320, 240);
	auto rect = src->GetRect();
	auto zoom_x = 2.0;
	auto zoom_y = 2.0;
	auto angle = M_PI;
	int waver_depth = 2;
	double waver_phase = M_PI;
	for (auto _: state) {
		dest->EffectsBlit(0, 0, 0, 0,
				*src, rect,
				opacity,
				zoom_x, zoom_y,
				angle,
				waver_depth,
				waver_phase);
	}
}

BENCHMARK(BM_EffectsBlit);



BENCHMARK_MAIN();


