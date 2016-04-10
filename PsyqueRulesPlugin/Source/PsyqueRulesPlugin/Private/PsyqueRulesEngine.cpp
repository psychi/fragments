// Copyright (c) 2016, Hillco Psychi, All rights reserved.

#include "PsyqueRulesPluginPrivatePCH.h"
#include "PsyqueRulesEngine.h"
#include "Psyque/RulesEngine/Driver.h"
#include "Templates/SharedPointer.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPsyqueRulesEngine, Log, All);
DEFINE_LOG_CATEGORY(LogPsyqueRulesEngine);

//-----------------------------------------------------------------------------
namespace
{
	using FPsyqueRulesEngineDriver = Psyque::RulesEngine::TDriver<
		uint64, float, int32, std::hash<FName>, std::allocator<void*>>;
	TSharedPtr<FPsyqueRulesEngineDriver> UnnamedDriver;

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
bool UPsyqueRulesEngine::CreateDriver()
{
	auto const LocalDriver(new FPsyqueRulesEngineDriver);
	if (LocalDriver != nullptr)
	{
		::UnnamedDriver = ::MakeShareable(LocalDriver);
		return true;
	}
	else {return false;}
}

bool UPsyqueRulesEngine::DestroyDriver()
{
	if (::UnnamedDriver.IsValid())
	{
		::UnnamedDriver.Reset();
		return true;
	}
	else {return false;}
}

//-----------------------------------------------------------------------------
void UPsyqueRulesEngine::Tick()
{
	::UnnamedDriver->Tick();
}

int32 UPsyqueRulesEngine::MakeHash(FName const& InName) const
{
	return ::UnnamedDriver->HashFunction(InName);
}

//-----------------------------------------------------------------------------
bool UPsyqueRulesEngine::RegisterBoolStatus(
	int32 const InChunkKey,
	int32 const InStatusKey,
	bool const InValue)
{
	return ::UnnamedDriver->RegisterStatus(InChunkKey, InStatusKey, InValue);
}

bool UPsyqueRulesEngine::RegisterUnsignedStatus(
	int32 const InChunkKey,
	int32 const InStatusKey,
	int32 const InValue,
	uint8 const InBitWidth)
{
	return 0 <= InValue
		&& ::UnnamedDriver->RegisterStatus(
			InChunkKey, InStatusKey, static_cast<uint32>(InValue), InBitWidth);
}

bool UPsyqueRulesEngine::RegisterSignedStatus(
	int32 const InChunkKey,
	int32 const InStatusKey,
	int32 const InValue,
	uint8 const InBitWidth)
{
	return InValue != ThisClass::GetIntegerNan()
		&& ::UnnamedDriver->RegisterStatus(
			InChunkKey, InStatusKey, InValue, InBitWidth);
}

bool UPsyqueRulesEngine::RegisterFloatStatus(
	int32 const InChunkKey,
	int32 const InStatusKey,
	float const InValue)
{
	return InValue != ThisClass::GetFloatNan()
		&& ::UnnamedDriver->RegisterStatus(InChunkKey, InStatusKey, InValue);
}

//-----------------------------------------------------------------------------
EPsyqueKleene UPsyqueRulesEngine::GetBoolStatus(int32 const InStatusKey) const
{
	auto const LocalValue(
		::UnnamedDriver->GetReservoir().FindStatus(InStatusKey));
	auto const LocalBool(LocalValue.GetBool());
	return LocalBool != nullptr?
		static_cast<EPsyqueKleene>(*LocalBool): EPsyqueKleene::TernaryUnknown;
}

int32 UPsyqueRulesEngine::GetUnsignedStatus(int32 const InStatusKey) const
{
	auto const LocalValue(
		::UnnamedDriver->GetReservoir().FindStatus(InStatusKey));
	auto const LocalUnsigned(LocalValue.GetUnsigned());
	if (LocalUnsigned != nullptr &&
		*LocalUnsigned <= static_cast<uint32>(std::numeric_limits<int32>::max()))
	{
		return static_cast<int32>(*LocalUnsigned);
	}
	return ThisClass::GetIntegerNan();
}

int32 UPsyqueRulesEngine::GetSignedStatus(int32 const InStatusKey) const
{
	auto const LocalValue(
		::UnnamedDriver->GetReservoir().FindStatus(InStatusKey));
	auto const LocalSigned(LocalValue.GetSigned());
	if (LocalSigned != nullptr
		&& *LocalSigned <= std::numeric_limits<int32>::max()
		&& ThisClass::GetIntegerNan() < *LocalSigned)
	{
		return static_cast<int32>(*LocalSigned);
	}
	return ThisClass::GetIntegerNan();
}

float UPsyqueRulesEngine::GetFloatStatus(int32 const InStatusKey) const
{
	auto const LocalValue(
		::UnnamedDriver->GetReservoir().FindStatus(InStatusKey));
	auto const LocalFloat(LocalValue.GetFloat());
	return LocalFloat != nullptr? *LocalFloat: ThisClass::GetFloatNan();
}

//-----------------------------------------------------------------------------
bool UPsyqueRulesEngine::SetBoolStatus(
	int32 const InStatusKey,
	bool const InValue)
{
	return ::UnnamedDriver->AssignStatus(InStatusKey, InValue);
}

bool UPsyqueRulesEngine::SetUnsignedStatus(
	int32 const InStatusKey,
	int32 const InValue)
{
	return 0 <= InValue
		&& ::UnnamedDriver->AssignStatus(
			InStatusKey, static_cast<uint32>(InValue));
}

bool UPsyqueRulesEngine::SetSignedStatus(
	int32 const InStatusKey,
	int32 const InValue)
{
	return InValue != ThisClass::GetIntegerNan()
		&& ::UnnamedDriver->AssignStatus(InStatusKey, InValue);
}

bool UPsyqueRulesEngine::SetFloatStatus(
	int32 const InStatusKey,
	float const InValue)
{
	return InValue != ThisClass::GetIntegerNan()
		&& ::UnnamedDriver->AssignStatus(InStatusKey, InValue);
}

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
	::UnnamedDriver->ExtendChunk(
		InChunkKey,
		Psyque::RulesEngine::TStatusBuilder(),
		*InStatusTable,
		Psyque::RulesEngine::TExpressionBuilder(),
		*InExpressionTable,
		Psyque::RulesEngine::TStatusBuilder(),
		*InBehaviorTable);
}

//-----------------------------------------------------------------------------
void UPsyqueRulesEngine::ExtendChunkFromJsonString(
	int32 const InChunkKey,
	FString const& InStatusJson,
	FString const& InExpressionJson,
	FString const& InBehaviorJson)
{
	::UnnamedDriver->ExtendChunk(
		InChunkKey,
		Psyque::RulesEngine::TStatusBuilder(),
		StringToJsonArray(InStatusJson),
		Psyque::RulesEngine::TExpressionBuilder(),
		StringToJsonArray(InExpressionJson),
		Psyque::RulesEngine::TStatusBuilder(),
		StringToJsonArray(InBehaviorJson));
}

void UPsyqueRulesEngine::RemoveChunk(int32 const InChunkKey)
{
	::UnnamedDriver->RemoveChunk(InChunkKey);
}

// vim: set noexpandtab:
