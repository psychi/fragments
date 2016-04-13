// Copyright (c) 2016, Hillco Psychi, All rights reserved.

#include "PsyqueRulesPluginPrivatePCH.h"
#include "PsyqueRulesEngine.h"

//-----------------------------------------------------------------------------
class FPsyqueRulesPlugin: public IPsyqueRulesPlugin
{
	public:
	using ThisClass = FPsyqueRulesPlugin;
	using Super = IPsyqueRulesPlugin;

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FPsyqueRulesPlugin, PsyqueRulesPlugin)

//-----------------------------------------------------------------------------
void FPsyqueRulesPlugin::StartupModule()
{
	// This code will execute after your module is loaded into memory.
	// (but after global variables are initialized, of course.)
	// このモジュールがメモリにロードされた後で、この関数が呼び出される。
	// (もちろん、大域変数は初期化された後)
	this->Super::StartupModule();

	// ドライバを構築する。
	UPsyqueRulesEngine::Create();
}

void FPsyqueRulesPlugin::ShutdownModule()
{
	// ドライバを破棄する。
	UPsyqueRulesEngine::Destroy();

	// This function may be called during shutdown to clean up your module.
	// For modules that support dynamic reloading,
	// we call this function before unloading the module.
	// モジュールをクリーンアップするため、
	// シャットダウン時にこの関数が呼び出される。
	// 動的再読み込みをサポートするモジュールについては、
	// モジュールをアンロードする前にこの関数が呼び出される。
	this->Super::ShutdownModule();
}

// vim: set noexpandtab:
