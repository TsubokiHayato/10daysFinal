#pragma once
#include "Object3d.h"
#include "Vector3.h"
#include "Vector4.h"
#include <memory>
#include <vector>

namespace TuboEngine { class Camera; }

// =============================================================================
//  TitleItemRain ── タイトル画面で「ゲーム内オブジェクトが降り続ける」背景演出。
//
//  ・ゲーム本編で使われている全モデル（block/square/cone/drill/star/crown/
//    playerBullet/player/artilleryBattery/wall/tile）を上空から降らせる。
//  ・固定数のプール(drops_)を使い回す。下端(kKillY)を割ったら上空へ戻すだけなので、
//    生成/破棄は起きず軽い。各 Drop にはモデルを固定割り当てし、再Initialize
//    （モデル再ロード）は行わない。全種を見せるのは初期割り当てで担保する。
//
//  使い方（TitleScene 側）:
//    Initialize()   : itemRain_->Initialize(camera_.get());
//    Update()       : itemRain_->Update(dt);
//    Object3DDraw() : itemRain_->Draw();
//    Finalize()     : itemRain_.reset();
// =============================================================================
class TitleItemRain {
public:
	// camera : タイトルの固定カメラ。count : 同時に降らせる個数。
	void Initialize(TuboEngine::Camera* camera, int count = 24);
	void Update(float dt);
	void Draw();

private:
	// 1粒（落下する1オブジェクト）。
	struct Drop {
		std::unique_ptr<TuboEngine::Object3d> obj;
		TuboEngine::Math::Vector3 pos{};
		TuboEngine::Math::Vector3 rot{};
		TuboEngine::Math::Vector3 spinVel{}; // 毎秒の回転量
		float fallSpeed = 0.0f;              // 毎秒の落下量
	};

	// 1粒を上空へ再配置する（落下速度・回転・XZ位置を抽選し直す）。
	//  aboveTop=true  : 画面上端より上へ（下端を割って戻すとき）。
	//  aboveTop=false : 画面内のランダムな高さへ（開幕から降っている状態にするとき）。
	void Respawn(Drop& d, bool aboveTop);

	std::vector<Drop> drops_;
	TuboEngine::Camera* camera_ = nullptr;
};
