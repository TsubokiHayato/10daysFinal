#pragma once
#include "Object3d.h"     // 3D モデル描画

namespace TuboEngine { class Camera; }

// =============================================================================
//  Cannon ── 砲台のクラス
//
//  ・SPACEで弾を発射することができる
//
// =============================================================================
namespace game {

	class Cannon {
	public:
		// camera : 描画に使う主カメラ
		void Initialize(TuboEngine::Camera* camera);
		void Update();
		void Draw();
		void ImGuiDraw();

		// Getter
		TuboEngine::Object3d* GetCannon() const { return cannon_.get(); }
		TuboEngine::Object3d* GetBullet() const { return bullet_; }
		bool GetIsBulletFired() const { return isBulletFired_; }

		// Setter	
		void SetCannon(std::unique_ptr<TuboEngine::Object3d> cannon) { cannon_ = std::move(cannon); }
		void SetBullet(TuboEngine::Object3d* bullet) { bullet_ = bullet; }
		void SetIsBulletFired(bool isBulletFired) { isBulletFired_ = isBulletFired; }

	private:
		// ───────── 3D オブジェクト ─────────
		// 発射台
		std::unique_ptr<TuboEngine::Object3d> cannon_;
		// セットする弾
		TuboEngine::Object3d* bullet_ = nullptr;
		// 弾が発射中かどうか
		bool isBulletFired_ = false;
	};
}