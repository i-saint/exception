#include "../interface.h"

namespace exception {

  obj_ptr CreateTitleBackground();
  gobj_ptr CreateBoss1();
  gobj_ptr CreateBoss2();
  gobj_ptr CreateBoss3();
  gobj_ptr CreateBoss4();
  gobj_ptr CreateBoss5();
  gobj_ptr CreateBossEx();

  gobj_ptr CreateTestStage1();
  gobj_ptr CreateStage1();
  gobj_ptr CreateStage2();
  gobj_ptr CreateStage3();
  gobj_ptr CreateStage4();
  gobj_ptr CreateStage5();
  gobj_ptr CreateStageEx();

  gobj_ptr CreateGameOver();
  gobj_ptr CreateStageResult(int bosstime, int hit, float score);


  void PutSmallExplode(const vector4& pos, int num, float strength=1.5f);
  void PutSmallExplode(const box& box, const matrix44& mat, int num, float strength=1.5f);
  void PutFlash(const vector4& pos, float size=100.0f);
  void PutCubeExplode(const vector4& pos);
  void PutSmallImpact(const vector4& pos);
  void PutMediumImpact(const vector4& pos);
  void PutBloom();
  void PutDistortion(const vector4& pos, const vector4& dir);

  gobj_ptr GetGlobals();
  player_ptr CreatePlayer(int cid, const string& name, IInput *input);
  fraction_ptr CreateFraction();


  void ResetGlobals();
  void SetGlobalScroll(const vector4& v);
  void SetGlobalAccel(const vector4& v);
  void SetGlobalMatrix(const matrix44& v);
  void SetGlobalBoundBox(const box& v);
  void SetGlobalBoundRect(const rect& v);

  const vector4& GetGlobalScroll();
  const vector4& GetGlobalAccel();
  const matrix44& GetGlobalMatrix();
  const matrix44& GetGlobalIMatrix();
  const box& GetGlobalBoundBox();
  const rect& GetGlobalBoundRect();

};
