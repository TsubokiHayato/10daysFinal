#pragma once
#include "Object3d.h"
#include "Vector3.h"
#include <cstdlib>
#include <vector>

// =============================================================================
//  Collapser ── 登録した Object3d 群を「崩落」（落下＋回転）させる小道具。
//
//  フィールドの床(パネル)だけでなく、その陣地に乗っている他のオブジェクト
//  （大砲・作業台・ベルトコンベア・アイテムなど）もまとめて崩す用途に使う。
//
//  使い方:
//    ・崩落させたい Object3d* を Add() で登録しておく。
//    ・Start() で各オブジェクトに落下速度と回転を割り当てて開始（多重呼び出し安全）。
//    ・毎フレーム Update() を呼ぶと落下＋回転を適用し、各 obj->Update() も行う。
//  ※ 崩落中はそのオブジェクトの通常 Update を呼ばず、Collapser 側に任せること。
// =============================================================================
namespace game {

class Collapser {
public:
	void Add(TuboEngine::Object3d* obj) { if (obj) targets_.push_back(obj); }
	bool Empty() const { return targets_.empty(); }
	bool Started() const { return started_; }

	// 落下パラメータを割り当てて崩落開始（すでに開始済みなら何もしない）。
	void Start() {
		if (started_) return;
		started_ = true;
		vel_.resize(targets_.size());
		spin_.resize(targets_.size());
		for (size_t i = 0; i < targets_.size(); ++i) {
			vel_[i] = {R11() * 0.08f, -0.04f - Pos01() * 0.20f, R11() * 0.08f};
			spin_[i] = {R11() * 0.10f, R11() * 0.06f, R11() * 0.10f};
		}
	}

	// 毎フレーム：重力で落下＋回転を適用する。
	void Update() {
		if (!started_) return;
		for (size_t i = 0; i < targets_.size(); ++i) {
			vel_[i].y -= 0.015f; // 重力
			TuboEngine::Object3d* o = targets_[i];
			o->SetPosition(o->GetPosition() + vel_[i]);
			o->SetRotation(o->GetRotation() + spin_[i]);
			o->Update();
		}
	}

private:
	static float Pos01() { return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX); }
	static float R11() { return Pos01() * 2.0f - 1.0f; }

	std::vector<TuboEngine::Object3d*> targets_;
	std::vector<TuboEngine::Math::Vector3> vel_;
	std::vector<TuboEngine::Math::Vector3> spin_;
	bool started_ = false;
};

} // namespace game
