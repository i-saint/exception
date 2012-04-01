#ifndef version_h
#define version_h

#ifndef _WIN32_WINNT
  #define _WIN32_WINNT 0x0500
#endif
#ifndef WINVER
  #define WINVER 0x0500
#endif

// トライアルバージョン(2,4,5面無し)にするか 
//#define EXCEPTION_TRIAL

#define EXCEPTION_REPLAY_VERSION 106
#define EXCEPTION_VERSION 111

#ifdef EXCEPTION_DEBUG
  #define EXCEPTION_ENABLE_PROFILE          // プロファイルダイアログの有無 
  #define EXCEPTION_ENABLE_SCENE_EDIT       // シーンセレクトの有無 
  #define EXCEPTION_ENABLE_STATE_SAVE       // ステートセーブ機能の有無 
  #define EXCEPTION_ENABLE_DATA_RESCUE      // Win32Exceptionが飛んだ時もリプレイとかを保存する。/EHa必須。遅くなる 
//  #define EXCEPTION_ENABLE_RUNTIME_CHECK    // 各種ランタイムチェックの有無。かなり遅くなる 
#else
#endif

#ifdef WIN32
  #define EXCEPTION_ENABLE_NETRANKING // ネットランキングの有無 
  #define EXCEPTION_ENABLE_NETPLAY    // ネット越し複数人プレイの有無 
  #ifndef EXCEPTION_TRIAL
    #define EXCEPTION_ENABLE_NETUPDATE // 自動アップデートの有無 
  #endif
#endif


#endif
