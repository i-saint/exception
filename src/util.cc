#include "stdafx.h"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>

namespace exception {

#ifdef EXCEPTION_ENABLE_RUNTIME_CHECK
  void ErrorIfAsyncPhase()
  {
    if(!GetGame()->isSynchronized()) {
      throw Error("ErrorIfAsyncPhase()");
    }
  }

  void ErrorIfSyncPhase()
  {
    if(GetGame()->isSynchronized()) {
      throw Error("ErrorIfSyncPhase()");
    }
  }
#endif // EXCEPTION_ENABLE_RUNTIME_CHECK 

  void PushLinkage(Deserializer& s)
  {
    gid id;
    s >> id;
    GetLinkageInfo().push(id);
  }

  void PushLinkage(gid id)
  {
    GetLinkageInfo().push(id);
  }

  gid PopLinkageID()
  {
    linkage_info& li = GetLinkageInfo();
    gid id = li.front();
    li.pop();
    return id;
  }

  gobj_ptr PopLinkage()
  {
    return GetObjectByID(PopLinkageID());
  }


  gid ToID(gobj_ptr p) { return p ? p->getID() : 0; }

  bool IsSolid(gobj_ptr p)  { return p && (p->getType() & GameObject::SOLID)!=0; }
  bool IsEnemy(gobj_ptr p)  { return p && (p->getType() & GameObject::ENEMY)!=0; }
  bool IsFraction(gobj_ptr p){return p && (p->getType() & GameObject::FRACTION)!=0; }
  bool IsPlayer(gobj_ptr p) { return p && (p->getType() & GameObject::PLAYER)!=0; }
  bool IsGround(gobj_ptr p) { return p && (p->getType() & GameObject::GROUND)!=0; }
  bool IsBullet(gobj_ptr p) { return p && (p->getType() & GameObject::BULLET)!=0; }
  bool IsEffect(gobj_ptr p) { return p && (p->getType() & GameObject::EFFECT)!=0; }
  bool IsLayer(gobj_ptr p)  { return p && (p->getType() & GameObject::LAYER)!=0; }

  solid_ptr  ToSolid(gobj_ptr p) { return (IsSolid(p) ? static_cast<solid_ptr>(p) : 0); }
  enemy_ptr  ToEnemy(gobj_ptr p) { return (IsEnemy(p) ? static_cast<enemy_ptr>(p) : 0); }
  fraction_ptr ToFraction(gobj_ptr p) { return (IsFraction(p) ? static_cast<fraction_ptr>(p) : 0); }
  player_ptr ToPlayer(gobj_ptr p){ return (IsPlayer(p) ? static_cast<player_ptr>(p) : 0); }
  ground_ptr ToGround(gobj_ptr p){ return (IsGround(p) ? static_cast<ground_ptr>(p) : 0); }
  bullet_ptr ToBullet(gobj_ptr p){ return (IsBullet(p) ? static_cast<bullet_ptr>(p) : 0); }
  effect_ptr ToEffect(gobj_ptr p){ return (IsEffect(p) ? static_cast<effect_ptr>(p) : 0); }
  layer_ptr  ToLayer(gobj_ptr p) { return (IsLayer(p)  ? static_cast<layer_ptr>(p) : 0); }

  float GenRand()
  {
    static ist::Random rand(::time(0));
    return float(rand.genReal());
  }

  size_t GetChecksum(const string& path)
  {
    std::FILE *f = ::fopen(path.c_str(), "rb");
    if(!f) {
      return 0;
    }

    size_t sum = 0;
    char c = 0;
    while(::fread(&c, 1, 1, f)) {
      sum+=c;
    }
    return sum;
  }



  vector4 GetUnprojectedPosition(const vector2& screen)
  {
    double model[16];
    double proj[16];
    int view[4];
    double ox,oy,oz;

    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, view);

    gluUnProject(screen.x, sgui::View::instance()->getWindowSize().getHeight()-screen.y, 1.0f,
      model, proj, view, &ox, &oy, &oz);
    return vector4(float(ox), float(oy), float(oz));
  }

  vector2 GetProjectedPosition(const vector4& pos)
  {
    int view[4];
    double model[16], project[16];
    double dx, dy, dz;
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, project);
    glGetIntegerv(GL_VIEWPORT, view);
    gluProject(pos.x, pos.y, pos.z, model, project, view, &dx, &dy, &dz);
    dx*=(640.0/view[2]);
    dy*=(480.0/view[3]);
    return vector2(float(dx), float(480.0-dy));
  }

  bool IsInnerScreen(const vector4& pos, const vector2& expand)
  {
    return rect(vector2(640, 480)+expand, vector2(0, 0)-expand).isInner(GetProjectedPosition(pos));
  }

  void DrawRect(const vector2& ur, const vector2& bl, const vector2& tur, const vector2& tbl)
  {
    ScreenMatrix sm;

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glBegin(GL_QUADS);
    glTexCoord2f(tbl.x, tur.y);
    glVertex2f(bl.x, bl.y);
    glTexCoord2f(tbl.x, tbl.y);
    glVertex2f(bl.x, ur.y);
    glTexCoord2f(tur.x, tbl.y);
    glVertex2f(ur.x, ur.y);
    glTexCoord2f(tur.x, tur.y);
    glVertex2f(ur.x, bl.y);
    glEnd();
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
  }

  const float* CreateBoxVBO(const box& box)
  {
    vector3 vertex[24];
    vector3 normal[24];

    vector3 n;
    const vector4& ur = box.getUpperRight();
    const vector4& bl = box.getBottomLeft();

    n = vector3(1.0f, 0.0f, 0.0f);
    normal[0] = n;
    normal[1] = n;
    normal[2] = n;
    normal[3] = n;
    vertex[0] = vector3(ur.x, ur.y, ur.z);
    vertex[1] = vector3(ur.x, bl.y, ur.z);
    vertex[2] = vector3(ur.x, bl.y, bl.z);
    vertex[3] = vector3(ur.x, ur.y, bl.z);

    n = vector3(-1.0f, 0.0f, 0.0f);
    normal[4] = n;
    normal[5] = n;
    normal[6] = n;
    normal[7] = n;
    vertex[4] = vector3(bl.x, ur.y, ur.z);
    vertex[5] = vector3(bl.x, ur.y, bl.z);
    vertex[6] = vector3(bl.x, bl.y, bl.z);
    vertex[7] = vector3(bl.x, bl.y, ur.z);

    n = vector3(0.0f, 1.0f, 0.0f);
    normal[8] = n;
    normal[9] = n;
    normal[10]= n;
    normal[11]= n;
    vertex[8] = vector3(ur.x, ur.y, ur.z);
    vertex[9] = vector3(ur.x, ur.y, bl.z);
    vertex[10]= vector3(bl.x, ur.y, bl.z);
    vertex[11]= vector3(bl.x, ur.y, ur.z);

    n = vector3(0.0f, -1.0f, 0.0f);
    normal[12] = n;
    normal[13] = n;
    normal[14] = n;
    normal[15] = n;
    vertex[12] = vector3(ur.x, bl.y, ur.z);
    vertex[13] = vector3(bl.x, bl.y, ur.z);
    vertex[14] = vector3(bl.x, bl.y, bl.z);
    vertex[15] = vector3(ur.x, bl.y, bl.z);

    n = vector3(0.0f, 0.0f, 1.0f);
    normal[16] = n;
    normal[17] = n;
    normal[18] = n;
    normal[19] = n;
    vertex[16] = vector3(ur.x, ur.y, ur.z);
    vertex[17] = vector3(bl.x, ur.y, ur.z);
    vertex[18] = vector3(bl.x, bl.y, ur.z);
    vertex[19] = vector3(ur.x, bl.y, ur.z);

    n = vector3(0.0f, 0.0f, -1.0f);
    normal[20] = n;
    normal[21] = n;
    normal[22] = n;
    normal[23] = n;
    vertex[20] = vector3(ur.x, ur.y, bl.z);
    vertex[21] = vector3(ur.x, bl.y, bl.z);
    vertex[22] = vector3(bl.x, bl.y, bl.z);
    vertex[23] = vector3(bl.x, ur.y, bl.z);

    static float tmp[24*6];
    for(size_t i=0; i<24; ++i) {
      float *dst = tmp+(6*i);
      std::copy(normal[i].v, normal[i].v+3, dst+0);
      std::copy(vertex[i].v, vertex[i].v+3, dst+3);
    }
    return tmp;
  }


  void PlaySound(ISound *s, int ch)
  {
    static size_t last[8] = {0,0,0,0,0,0,0,0};
    static ISound *list[8] = {0,0,0,0,0,0,0,0};

    size_t frame = GetPast();
    if(list[ch]!=s || last[ch]!=frame) {
      list[ch] = s;
      last[ch] = frame;
      s->play(ch);
    }
  }

}
