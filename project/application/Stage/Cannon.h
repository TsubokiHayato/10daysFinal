#pragma once
#include "Object3d.h"     // 3D モデル描画
#include "Vector3.h"

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
		void Initialize(TuboEngine::Camera* camera, TuboEngine::Math::Vector3 propPos);
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
		void SetIsLoading(bool flag) { isLoading_ = flag; }

	private:
		// ───────── 3D オブジェクト ─────────
		// 発射台
		std::unique_ptr<TuboEngine::Object3d> cannon_;

		std::unique_ptr<TuboEngine::Object3d> iconModel_;

		TuboEngine::Math::Vector3 propPos_;

		float iconTimer_ = 0.0f;

		float iconMaxTime_ = 0.3f;

		// セットする弾
		TuboEngine::Object3d* bullet_ = nullptr;
		// 弾が発射中かどうか
		bool isBulletFired_ = false;

		bool isLoading_ = false;
	};
}