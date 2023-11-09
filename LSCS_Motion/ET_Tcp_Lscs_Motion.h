// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "Async/Async.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SocketSubsystem.h"
#include "Sockets.h"

#include "Misc/FileHelper.h"
#include "IPAddress.h"
#include "HAL/ThreadSafeBool.h"

#include "ET_Tcp_Lscs_Motion.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNetEventLSCSMotion);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetMsgEventLSCSMotion, const TArray<uint8>&, Bytes);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KOVC_LEVELPROTOTYPE_V01_API UET_Tcp_Lscs_Motion : public UActorComponent
{
	GENERATED_BODY()

public:	
	UET_Tcp_Lscs_Motion();

protected:
	virtual void BeginPlay() override;

	FSocket* ClientSocket;

	FThreadSafeBool bShouldReceiveData;
	
	TFuture<void> ClientConnectionFinishedFuture;

	TSharedPtr<FInternetAddr> RemoteAddress;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// packet's header datas
	UPROPERTY()
	FString protocolId;
	UPROPERTY()
	uint64 requestSequence;
	UPROPERTY()
	uint32 bodyLength;

	UPROPERTY()
	FString ClientSocketName;
	UPROPERTY()
	int32 BufferMaxSize;
	UPROPERTY()
	bool bIsConnected;

	UFUNCTION()
	uint64 ReadRequestSequence(const TArray<uint8>& bytes);

	UFUNCTION()
		TArray<uint8> fStringToBytes(FString InString);
	UFUNCTION()
		TArray<uint8> fUInt64ToBytes(uint64 InInt64);
	UFUNCTION()
		TArray<uint8> fUInt32ToBytes(uint32 InInt32);
	UFUNCTION()
		TArray<uint8> fInt32ToBytes(int32 InInt32);
	UFUNCTION()
		int32 fInt32HtoN(int32 fVal);
	UFUNCTION()
		uint32 fUInt32HtoN(uint32 fVal);
	UFUNCTION()
		uint32 ReadBodylength(const TArray<uint8>& Bytes);
	UFUNCTION()
	void CloseSocket();
	UPROPERTY()
		TArray<uint8> Receive_Global;
	UPROPERTY(BlueprintReadWrite, BlueprintAssignable, BlueprintCallable)
	FNetEventLSCSMotion OnConnected;

	UPROPERTY(BlueprintReadWrite, BlueprintAssignable, BlueprintCallable)
	FNetMsgEventLSCSMotion OnReceivedBytes;

	UFUNCTION(BlueprintCallable, Category = "ETRI LSCS Motion")
	void ConnectToLSCSMotion(const FString& InIP = TEXT("115.94.201.90"), const int32 InPort = 26005);

	UFUNCTION(BlueprintCallable, Category = "ETRI LSCS Motion")
	FString ReadProtocolId(const TArray<uint8>& Bytes);

	UFUNCTION(BlueprintCallable, Category = "ETRI LSCS Motion")
	int32 ReadTimestamp(const TArray<uint8>& Bytes);

	UFUNCTION(BlueprintCallable, Category = "ETRI LSCS Motion")
	int32 ReadActorIndex(const TArray<uint8>& Bytes);

	UFUNCTION(BlueprintCallable, Category = "ETRI LSCS Motion")
	int32 ReadJointCount(const TArray<uint8>& Bytes);

	UFUNCTION(BlueprintCallable, Category = "ETRI LSCS Motion")
	void ReadMotionDataFloats(const TArray<uint8>& Bytes, TArray<float>& MotionData, int32& timestamp, int32& actor_index, int32& joint_count);
	UFUNCTION(BlueprintCallable, Category = "ETRI LSCS Motion")
	void ReadMotionDataRotates(const TArray<uint8>& Bytes, FVector& MotionPosition, TArray<FRotator>& MacroData, TArray<FRotator>& OriginData, int32& timestamp, int32& mesh_index, int32& actor_index, int32& joint_count);


	UFUNCTION(BlueprintCallable, Category = "ETRI LSCS")
	void DebugLog(const FString& InIP = TEXT("Message"));

	UFUNCTION(BlueprintCallable, Category = "ETRI LSCS Motion")
	void SendUpdateConnectionInfo(FString _protocolId, int32 _connectionNumber);
};
