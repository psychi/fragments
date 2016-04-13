// Copyright (c) 2016, Hillco Psychi, All rights reserved.

#include "PsyqueRulesPluginPrivatePCH.h"
#include "PsyqueRulesEngine.h"
#include "UObject/WeakObjectPtrTemplates.h"

DEFINE_LOG_CATEGORY(LogPsyqueRulesEngine);

//-----------------------------------------------------------------------------
namespace
{
	TWeakObjectPtr<UPsyqueRulesEngine> UnnamedEngine;

	TArray<TSharedPtr<FJsonValue>> StringToJsonArray(
		FString const& InJsonString)
	{
		auto const LocalJsonReader(
			StaticCastSharedRef<TJsonReader<TCHAR>>(
				FJsonStringReader::Create(InJsonString)));
		TArray<TSharedPtr<FJsonValue>> LocalJsonArray;
		if (!FJsonSerializer::Deserialize(LocalJsonReader, LocalJsonArray))
		{
			UE_LOG(
				LogPsyqueRulesEngine,
				Warning,
				TEXT(
					"FJsonSerializer::Deserialize is failed"
					" in PsyqueRulesPlugin/StringToJsonArray"));
			LocalJsonArray.Empty();
		}
		return LocalJsonArray;
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
UPsyqueRulesEngine::UPsyqueRulesEngine(
	FObjectInitializer const& InObjectInitializer):
Super(InObjectInitializer)
{}

//-----------------------------------------------------------------------------
void UPsyqueRulesEngine::ExtendChunkFromDataTable(
	int32 const InChunkKey,
	UDataTable const* const InStatusTable,
	UDataTable const* const InExpressionTable,
	UDataTable const* const InBehaviorTable)
{
	if (InStatusTable == nullptr)
	{
		UE_LOG(
			LogPsyqueRulesEngine,
			Error,
			TEXT("InStatusTable is nullptr in %s."),
			__func__);
		return;
	}
	if (InExpressionTable == nullptr)
	{
		UE_LOG(
			LogPsyqueRulesEngine,
			Error,
			TEXT("InExpressionTable is nullptr in %s."),
			__func__);
		return;
	}
	if (InBehaviorTable == nullptr)
	{
		UE_LOG(
			LogPsyqueRulesEngine,
			Error,
			TEXT("InBehaviorTable is nullptr in %s."),
			__func__);
		return;
	}
	this->Driver.ExtendChunk(
		InChunkKey,
		Psyque::RulesEngine::TStatusBuilder(),
		*InStatusTable,
		Psyque::RulesEngine::TExpressionBuilder(),
		*InExpressionTable,
		Psyque::RulesEngine::TStatusBuilder(),
		*InBehaviorTable);
}

void UPsyqueRulesEngine::ExtendChunkFromJsonString(
	int32 const InChunkKey,
	FString const& InStatusJson,
	FString const& InExpressionJson,
	FString const& InBehaviorJson)
{
	this->Driver.ExtendChunk(
		InChunkKey,
		Psyque::RulesEngine::TStatusBuilder(),
		StringToJsonArray(InStatusJson),
		Psyque::RulesEngine::TExpressionBuilder(),
		StringToJsonArray(InExpressionJson),
		Psyque::RulesEngine::TStatusBuilder(),
		StringToJsonArray(InBehaviorJson));
}

//-----------------------------------------------------------------------------
UPsyqueRulesEngine* UPsyqueRulesEngine::Create()
{
	auto const LocalEngine(NewObject<UPsyqueRulesEngine>());
	if (LocalEngine != nullptr)
	{
		::UnnamedEngine = LocalEngine;
		return LocalEngine;
	}
	return nullptr;
}

bool UPsyqueRulesEngine::Destroy()
{
	if (::UnnamedEngine.IsValid())
	{
		::UnnamedEngine.Reset();
		return true;
	}
	return false;
}

UPsyqueRulesEngine* UPsyqueRulesEngine::Get()
{
	return ::UnnamedEngine.Get();
}
// vim: set noexpandtab:
