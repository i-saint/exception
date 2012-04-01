#ifndef enemy_util_h
#define enemy_util_h

namespace exception {


  // 難度別パラメータ 
  inline int GetBossBonusRate()
  {
    const int v[] = {20, 50, 100, 200, 800};
    return v[GetLevel()];
  }

  inline float GetFractionRate()
  {
    const float v[] = {10.0f, 18.0f, 30.0f, 48.0f, 90.0f};
    return v[GetLevel()];
  }

  inline float GetFractionDamage()
  {
    const float v[] = {4.0f, 2.7f, 2.0f, 1.4f, 0.9f};
    return v[GetLevel()];
  }

  inline float GetMaxEnergy()
  {
    const float v[] = {80.0f, 130.0f, 180.0f, 230.0f, 280.0f};
    return v[GetLevel()];
  }


  // モーション補間関数郡 
  // v=0.0～1.0 
  inline float Sin90(float v)  { return sinf(90.0f*v*ist::radian); }
  inline float Sin90I(float v) { return 1.0f-sinf(90.0f*v*ist::radian); }
  inline float Cos90(float v)  { return cosf(90.0f*v*ist::radian); }
  inline float Cos90I(float v) { return 1.0f-cosf(90.0f*v*ist::radian); }
  inline float Cos180(float v) { return cosf(180.0f*v*ist::radian)/2.0f+0.5f; }
  inline float Cos180I(float v){ return 1.0f-(cosf(180.0f*v*ist::radian)/2.0f+0.5f); }

  // -1～1の乱数 
  float GetRand2() { return GetRand()*2.0f-1.0f; }


  inline player_ptr GetNearestPlayer(const vector4& pos)
  {
    player_ptr r = 0;
    float d = 0;
    for(size_t i=0; i<GetPlayerCount(); ++i) {
      player_ptr pl = GetPlayer(i);
      if(!pl) {
        continue;
      }

      float td = (pl->getPosition()-pos).norm();
      if(!r || td<d) {
        r = pl;
        d = td;
      }
    }
    return r;
  }

  inline vector4 GetNearestPlayerPosition(const vector4& pos)
  {
    if(player_ptr pl = GetNearestPlayer(pos)) {
      return pl->getPosition();
    }
    else {
      return GetPlayerPosition(0);
    }
  }

  inline void InvincibleAllPlayers(int frame)
  {
    for(size_t i=0; i<GetPlayerCount(); ++i) {
      if(player_ptr pl = GetPlayer(i)) {
        pl->setInvincible(frame);
      }
    }
  }


  inline solid_ptr GetParentSolid(gobj_ptr p)
  {
    solid_ptr gp = 0;
    while(p) {
      if(gp=ToSolid(p)) {
        break;
      }
      p = p->getParent();
    }
    return gp;
  }

  inline layer_ptr GetParentLayer(gobj_ptr p)
  {
    layer_ptr gp = 0;
    while(p) {
      if(gp=ToLayer(p)) {
        break;
      }
      p = p->getParent();
    }
    return gp;
  }

  inline void Blow(IThreadSpecificMethod& tsm, const vector4& center, float radius, float strength)
  {
    cdetector cd;
    sphere s;
    s.setPosition(center);
    s.setRadius(radius);
    gobj_iter& it = tsm.getObjects(s);
    while(it.has_next()) {
      gobj_ptr p = it.iterate();
      const vector4& tp = p->getPosition();
      if(s.isInner(tp)) {
        vector4 r = (tp-center);
        float l = r.norm();
        vector4 n = r/l;
        float str = std::max<float>(strength-l/radius, 0);
        tsm.sendAccelMessage(0, p, vector4(n*str));
      }
    }
  }


  inline void SplinkleCube(const box& box, const matrix44& mat, int num)
  {
    for(int i=0; i<num; ++i) {
      fraction_ptr p = CreateFraction();
      vector4 pos = box.getBottomLeft() + vector4(box.getWidth()*GetRand(), box.getHeight()*GetRand(), box.getLength()*GetRand());
      p->setPosition(mat*pos);
    }
  }

  inline void DestroyAllEnemy(IThreadSpecificMethod& tsm)
  {
    gobj_iter& i = tsm.getAllObjects();
    while(i.has_next()) {
      if(enemy_ptr p = ToEnemy(i.iterate())) {
        tsm.sendDestroyMessage(0, p, 1);
      }
    }
  }

  inline void KillIfOutOfScreen(gobj_ptr p, const rect& space)
  {
    rect bound(vector2(640, 480)+space.getUpperRight(), vector2()+space.getBottomLeft());
    if(p && !bound.isInner(GetProjectedPosition(p->getPosition()))) {
      p->SendKillMessage(0, p);
    }
  }

  inline void KillIfOutOfBox(gobj_ptr p, const box& space)
  {
    if(p && !space.isInner(p->getPosition())) {
      p->SendKillMessage(0, p);
    }
  }

  inline void Scratch(gobj_ptr p, CollideMessage& m, float damage=1.0f)
  {
    solid_ptr from = ToSolid(m.getFrom());
    if((IsEnemy(from) || IsGround(from)) && !IsFraction(from)) {
      p->SendDamageMessage(from, p, damage);
    }
  }


  template<class Ptr>
  inline void Destroy(IThreadSpecificMethod& tsm, Ptr &v, int stat=0)
  {
    if(v) {
      tsm.sendDestroyMessage(0, v, stat);
    }
  }

  template<class Ptr, size_t N>
  inline void Destroy(IThreadSpecificMethod& tsm, Ptr (&v)[N], int stat=0)
  {
    for(size_t i=0; i<N; ++i) {
      if(v[i]) {
        tsm.sendDestroyMessage(0, v[i], stat);
      }
    }
  }


  template<class Ptr, size_t N>
  inline int AliveCount(Ptr (&v)[N])
  {
    int r = 0;
    for(size_t i=0; i<N; ++i) {
      if(v[i]) {
        ++r;
      }
    }
    return r;
  }


  template<class Ptr>
  inline bool AliveAny(Ptr &v)
  {
    if(v) {
      return true;
    }
    return false;
  }

  template<class Ptr, size_t N>
  inline bool AliveAny(Ptr (&v)[N])
  {
    for(size_t i=0; i<N; ++i) {
      if(v[i]) {
        return true;
      }
    }
    return false;
  }


  template<class Ptr>
  inline bool DeadAny(Ptr &v)
  {
    if(!v) {
      return true;
    }
    return false;
  }

  template<class Ptr, size_t N>
  inline bool DeadAny(Ptr (&v)[N])
  {
    for(size_t i=0; i<N; ++i) {
      if(!v[i]) {
        return true;
      }
    }
    return false;
  }


  template<class Ptr>
  inline void SetGroup(Ptr &v, gid group)
  {
    if(v) {
      v->setGroup(group);
    }
  }

  template<class Ptr, size_t N>
  inline void SetGroup(Ptr (&v)[N], gid group)
  {
    for(size_t i=0; i<N; ++i) {
      if(v[i]) {
        v[i]->setGroup(group);
      }
    }
  }


  template<class Ptr>
  inline void Unchain(Ptr &v)
  {
    if(v) {
      v->unchain();
    }
  }

  template<class Ptr, size_t N>
  inline void Unchain(Ptr (&v)[N])
  {
    for(size_t i=0; i<N; ++i) {
      if(v[i]) {
        v[i]->unchain();
      }
    }
  }
}
#endif
