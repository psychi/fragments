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
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
UPsyqueRulesEngine::UPsyqueRulesEngine(
	FObjectInitializer const& InObjectInitializer):
Super(InObjectInitializer)
{}

//-----------------------------------------------------------------------------
bool UPsyqueRulesEngine::CreateDriver()
{
	if (::UnnamedDriver.IsValid()) {return false;}
	else
	{
		::UnnamedDriver = ::MakeShareable(new FPsyqueRulesEngineDriver);
		auto const LocalValid(::UnnamedDriver.IsValid());
		check(LocalValid);
		return LocalValid;
	}
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
		&& InBitWidth < sizeof(int32) * CHAR_BIT
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
		&& InBitWidth <= sizeof(int32) * CHAR_BIT
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
	if (LocalUnsigned != nullptr)
	{
		auto const LocalInteger(static_cast<int32>(*LocalUnsigned));
		if (0 <= LocalInteger && LocalInteger == *LocalUnsigned)
		{
			return LocalInteger;
		}
	}
	return ThisClass::GetIntegerNan();
}

int32 UPsyqueRulesEngine::GetSignedStatus(int32 const InStatusKey) const
{
	auto const LocalValue(
		::UnnamedDriver->GetReservoir().FindStatus(InStatusKey));
	auto const LocalSigned(LocalValue.GetSigned());
	return LocalSigned != nullptr?
		static_cast<int32>(*LocalSigned): ThisClass::GetIntegerNan();
}

float UPsyqueRulesEngine::GetFloatStatus(int32 const InStatusKey) const
{
	auto const LocalValue(
		::UnnamedDriver->GetReservoir().FindStatus(InStatusKey));
	auto const LocalFloat(LocalValue.GetFloat());
	return LocalFloat != nullptr?
		static_cast<float>(*LocalFloat): ThisClass::GetFloatNan();
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
void UPsyqueRulesEngine::ExtendChunkByJson(
	int32 const InChunkKey,
	FString const& InStatusJson,
	FString const& InExpressionJson,
	FString const& InBehaviorJson)
{
	::UnnamedDriver->ExtendChunk(
		InChunkKey,
		Psyque::RulesEngine::TStatusBuilder(),
		InStatusJson,
		Psyque::RulesEngine::TStatusBuilder(),
		InExpressionJson,
		Psyque::RulesEngine::TStatusBuilder(),
		InBehaviorJson);
}

void UPsyqueRulesEngine::RemoveChunk(int32 const InChunkKey)
{
	::UnnamedDriver->RemoveChunk(InChunkKey);
}

// vim: set noexpandtab:
