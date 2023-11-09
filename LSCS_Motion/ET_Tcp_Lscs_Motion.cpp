// Fill out your copyright notice in the Description page of Project Settings.


#include "ET_Tcp_Lscs_Motion.h"
#include "Memory.h"

UET_Tcp_Lscs_Motion::UET_Tcp_Lscs_Motion()
{
	PrimaryComponentTick.bCanEverTick = true;

	ClientSocketName = FString(TEXT("ue4-tcp-lscs-motion-client"));

	ClientSocket = nullptr;

	BufferMaxSize = 2 * 1024 * 1024;

	protocolId = "";
	requestSequence = 0;
	bodyLength = 0;
}

void UET_Tcp_Lscs_Motion::BeginPlay()
{
	Super::BeginPlay();
}

void UET_Tcp_Lscs_Motion::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UET_Tcp_Lscs_Motion::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Close LSCS - Motion."));

	CloseSocket();
}

void UET_Tcp_Lscs_Motion::ConnectToLSCSMotion(const FString& InIP, const int32 InPort)
{
	int count = 0;
	RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();

	UE_LOG(LogTemp, Error, TEXT("Try to connect to LSCS Motion <%s:%d>"), *InIP, InPort);
	GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, FString(TEXT("LSCS Motion address try to connect.")));

	bool bIsValid;
	RemoteAddress->SetIp(*InIP, bIsValid);
	RemoteAddress->SetPort(InPort);

	if (!bIsValid)
	{
		UE_LOG(LogTemp, Error, TEXT("LSCS Motion address is invalid <%s:%d>"), *InIP, InPort);
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(TEXT("LSCS Motion address is invalid.")));
		return;
	}

	ClientSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, ClientSocketName, false);

	ClientSocket->SetSendBufferSize(BufferMaxSize, BufferMaxSize);
	ClientSocket->SetReceiveBufferSize(BufferMaxSize, BufferMaxSize);

	bIsConnected = ClientSocket->Connect(*RemoteAddress);

	if (bIsConnected)
	{
		UE_LOG(LogTemp, Error, TEXT("LSCS Motion connected. <%s:%d>"), *InIP, InPort);
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Blue, FString(TEXT("LSCS Motion connected.")));
		OnConnected.Broadcast();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LSCS Motion connect failed. <%s:%d>"), *InIP, InPort);
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, FString(TEXT("LSCS Motion connect failed.")));
	}

	bShouldReceiveData = true;

	ClientConnectionFinishedFuture = Async(EAsyncExecution::Thread, [&]()
		{
			uint32 BufferSize = 0;
			TArray<uint8> ReceiveBuffer;
			FString ResultString;

			while (bShouldReceiveData)
			{
				if (ClientSocket->HasPendingData(BufferSize))
				{
					Receive_Global.SetNumUninitialized(BufferSize);

					int32 Read = 0;
					UE_LOG(LogTemp, Warning, TEXT("Before Receive Motion... ... ..."));
					ClientSocket->Recv(Receive_Global.GetData(), Receive_Global.Num(), Read);
					UE_LOG(LogTemp, Warning, TEXT("After Receive Motion... ... ..."));

					int32 messageLength = (int32)(ReadBodylength(Receive_Global) + 34);  //messageLength = header(34 bytes) + body
					//OnReceivedBytes.Broadcast(ReceiveBuffer);


					while (messageLength <= Receive_Global.Num() && messageLength > 0)
					{
						UE_LOG(LogTemp, Warning, TEXT("Before BroadCast Motion... ... ..."));
						OnReceivedBytes.Broadcast(Receive_Global);
						UE_LOG(LogTemp, Warning, TEXT("After BroadCast Motion... ... ..."));
						//count++;

						//UE_LOG(LogTemp, Error, TEXT("Receiving Motion ... ... ... %d"), count);
						if (messageLength >= Receive_Global.Num())
							break;

						for (int i = 0; i < messageLength; i++)
							Receive_Global.RemoveAt(0);

						messageLength = (int32)(ReadBodylength(Receive_Global) + 34);  //messageLength = header(34 bytes) + body

						//FPlatformProcess::Sleep(.01f);
					}
				}
				
				ClientSocket->Wait(ESocketWaitConditions::WaitForReadOrWrite, FTimespan(1));
			}
		});
}

uint32 UET_Tcp_Lscs_Motion::ReadBodylength(const TArray<uint8>& Bytes)
{
	uint32_t bl = (int32)(Bytes.GetData()[30] << 24 | Bytes.GetData()[31] << 16 | Bytes.GetData()[32] << 8 | Bytes.GetData()[33]);

	return bl;
}

FString UET_Tcp_Lscs_Motion::ReadProtocolId(const TArray<uint8>& Bytes)
{
	UE_LOG(LogTemp, Warning, TEXT("Before Read ProtocolID ... ... ... "));
	FString recvProtocolId;
	FFileHelper::BufferToString(recvProtocolId, Bytes.GetData(), Bytes.Num());
	UE_LOG(LogTemp, Warning, TEXT("After Read ProtocolID ... ... ... "));

	return recvProtocolId;
}

int32 UET_Tcp_Lscs_Motion::ReadTimestamp(const TArray<uint8>& Bytes)
{
	int32 timestamp = Bytes.GetData()[34] << 24 | Bytes.GetData()[35] << 16 | Bytes.GetData()[36] << 8 | Bytes.GetData()[37];


	return timestamp;
}

int32 UET_Tcp_Lscs_Motion::ReadActorIndex(const TArray<uint8>& Bytes)
{
	int16 actor_index = Bytes.GetData()[38] << 8 | Bytes.GetData()[39];

	return (int32)actor_index;
}

int32 UET_Tcp_Lscs_Motion::ReadJointCount(const TArray<uint8>& Bytes)
{
	int16 joint_count = Bytes.GetData()[52] << 8 | Bytes.GetData()[53];

	return (int32)joint_count;
}


void UET_Tcp_Lscs_Motion::ReadMotionDataFloats(const TArray<uint8>& Bytes, TArray<float>& MotionData, int32& timestamp, int32& actor_index, int32& joint_count)
{
	//TArray<float> result;
	//TimeStamp
	timestamp = Bytes.GetData()[34] << 24 | Bytes.GetData()[35] << 16 | Bytes.GetData()[36] << 8 | Bytes.GetData()[37];
	//ActorIndex
	int16 actor_index_data = Bytes.GetData()[38] << 8 | Bytes.GetData()[39];
	actor_index = (int32)actor_index_data;

	//JointCount
	int16 joint_count_data = Bytes.GetData()[52] << 8 | Bytes.GetData()[53];
	joint_count = (int32)joint_count_data;

	union {
		uint8_t mocap[4];
		float value;
	} res;

	int count = 0;

	float posX = 0;
	float posY = 0;
	float posZ = 0;

	res.mocap[0] = Bytes.GetData()[43];
	res.mocap[1] = Bytes.GetData()[42];
	res.mocap[2] = Bytes.GetData()[41];
	res.mocap[3] = Bytes.GetData()[40];
	posX = res.value;
	MotionData.Add(posX);
	res.mocap[0] = Bytes.GetData()[47];
	res.mocap[1] = Bytes.GetData()[46];
	res.mocap[2] = Bytes.GetData()[45];
	res.mocap[3] = Bytes.GetData()[44];
	posY = res.value;
	MotionData.Add(posY);
	res.mocap[0] = Bytes.GetData()[51];
	res.mocap[1] = Bytes.GetData()[50];
	res.mocap[2] = Bytes.GetData()[49];
	res.mocap[3] = Bytes.GetData()[48];
	posZ = res.value;
	MotionData.Add(posZ);


	for (int i = 0; i < (int)joint_count; i++)
	{
		float w = 0;
		float x = 0;
		float y = 0;
		float z = 0;

		res.mocap[0] = Bytes.GetData()[(i * 16) + 57];
		res.mocap[1] = Bytes.GetData()[(i * 16) + 56];
		res.mocap[2] = Bytes.GetData()[(i * 16) + 55];
		res.mocap[3] = Bytes.GetData()[(i * 16) + 54];
		w = res.value;
		MotionData.Add(w);
		res.mocap[0] = Bytes.GetData()[(i * 16) + 61];
		res.mocap[1] = Bytes.GetData()[(i * 16) + 60];
		res.mocap[2] = Bytes.GetData()[(i * 16) + 59];
		res.mocap[3] = Bytes.GetData()[(i * 16) + 58];
		x = res.value;
		MotionData.Add(x);
		res.mocap[0] = Bytes.GetData()[(i * 16) + 65];
		res.mocap[1] = Bytes.GetData()[(i * 16) + 64];
		res.mocap[2] = Bytes.GetData()[(i * 16) + 63];
		res.mocap[3] = Bytes.GetData()[(i * 16) + 62];
		y = res.value;
		MotionData.Add(y);
		res.mocap[0] = Bytes.GetData()[(i * 16) + 69];
		res.mocap[1] = Bytes.GetData()[(i * 16) + 68];
		res.mocap[2] = Bytes.GetData()[(i * 16) + 67];
		res.mocap[3] = Bytes.GetData()[(i * 16) + 66];
		z = res.value;
		MotionData.Add(z);
	}

	return;
}

void UET_Tcp_Lscs_Motion::ReadMotionDataRotates(const TArray<uint8>& Bytes, FVector& MotionPosition, TArray<FRotator>& MacroData, TArray<FRotator>& OriginData, int32& timestamp, int32& mesh_index, int32& actor_index, int32& joint_count)
{
	//TArray<float> result;

	UE_LOG(LogTemp, Warning, TEXT("Before Convert Motion... ... ..."));
	timestamp = Bytes.GetData()[34] << 24 | Bytes.GetData()[35] << 16 | Bytes.GetData()[36] << 8 | Bytes.GetData()[37];
	/*int16 actor_index_data = Bytes.GetData()[38] << 8 | Bytes.GetData()[39];
	actor_index = (int32)actor_index_data;*/
	mesh_index = (int32)Bytes.GetData()[38];
	actor_index = (int32)Bytes.GetData()[39];

	int16 joint_count_data = Bytes.GetData()[52] << 8 | Bytes.GetData()[53];
	joint_count = (int32)joint_count_data;
	FQuat macroquat;
	FQuat originquat;
	union {
		uint8_t mocap[4];
		float value;
	} res;

	int count = 0;

	float posX = 0;
	float posY = 0;
	float posZ = 0;

	res.mocap[0] = Bytes.GetData()[43];
	res.mocap[1] = Bytes.GetData()[42];
	res.mocap[2] = Bytes.GetData()[41];
	res.mocap[3] = Bytes.GetData()[40];
	posX = res.value;
	res.mocap[0] = Bytes.GetData()[47];
	res.mocap[1] = Bytes.GetData()[46];
	res.mocap[2] = Bytes.GetData()[45];
	res.mocap[3] = Bytes.GetData()[44];
	posY = res.value;
	res.mocap[0] = Bytes.GetData()[51];
	res.mocap[1] = Bytes.GetData()[50];
	res.mocap[2] = Bytes.GetData()[49];
	res.mocap[3] = Bytes.GetData()[48];
	posZ = res.value;
	MotionPosition.Set(posX, posY, posZ);

	for (int i = 0; i < (int)joint_count; i++)
	{
		float w = 0;
		float x = 0;
		float y = 0;
		float z = 0;

		res.mocap[0] = Bytes.GetData()[(i * 16) + 57];
		res.mocap[1] = Bytes.GetData()[(i * 16) + 56];
		res.mocap[2] = Bytes.GetData()[(i * 16) + 55];
		res.mocap[3] = Bytes.GetData()[(i * 16) + 54];
		w = res.value;

		res.mocap[0] = Bytes.GetData()[(i * 16) + 61];
		res.mocap[1] = Bytes.GetData()[(i * 16) + 60];
		res.mocap[2] = Bytes.GetData()[(i * 16) + 59];
		res.mocap[3] = Bytes.GetData()[(i * 16) + 58];
		x = res.value;
		res.mocap[0] = Bytes.GetData()[(i * 16) + 65];
		res.mocap[1] = Bytes.GetData()[(i * 16) + 64];
		res.mocap[2] = Bytes.GetData()[(i * 16) + 63];
		res.mocap[3] = Bytes.GetData()[(i * 16) + 62];
		y = res.value;
		res.mocap[0] = Bytes.GetData()[(i * 16) + 69];
		res.mocap[1] = Bytes.GetData()[(i * 16) + 68];
		res.mocap[2] = Bytes.GetData()[(i * 16) + 67];
		res.mocap[3] = Bytes.GetData()[(i * 16) + 66];
		z = res.value;
		/*macroquat.X = -1 * x;
		macroquat.Y = z;
		macroquat.Z = y;
		macroquat.W = w;*/
		macroquat.X = -w;
		macroquat.Y = x;
		macroquat.Z = -y;
		macroquat.W = z;
		
		originquat.X = x;
		originquat.Y = y;
		originquat.Z = z;
		originquat.W = w;
		MacroData.Add(macroquat.Rotator());
		OriginData.Add(originquat.Rotator());
	}

	UE_LOG(LogTemp, Warning, TEXT("After Convert Motion... ... ..."));
	return;
}

void UET_Tcp_Lscs_Motion::CloseSocket()
{
	if (ClientSocket)
	{
		bShouldReceiveData = false;

		ClientConnectionFinishedFuture.Get();
		ClientSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ClientSocket);
		ClientSocket = nullptr;
	}
}

void UET_Tcp_Lscs_Motion::DebugLog(const FString& InIP)
{
	UE_LOG(LogTemp, Error, TEXT("%s"), *InIP);
}


void UET_Tcp_Lscs_Motion::SendUpdateConnectionInfo(FString _protocolId, int32 _connectionNumber)
{
	bool resp = false;
	protocolId = _protocolId;	// ACCS0006
	FString requestTime = "20221122134525";
	bodyLength = 0;

	if (ClientSocket && ClientSocket->GetConnectionState() == SCS_Connected)
	{
		int32 BytesSent = 0;
		TArray<uint8> Bytes;

		TArray<uint8> cnBytes = fInt32ToBytes(_connectionNumber);

		bodyLength = cnBytes.Num();

		// Header
		Bytes.Append(fStringToBytes(protocolId));
		Bytes.Append(fUInt64ToBytes(requestSequence++));
		Bytes.Append(fStringToBytes(requestTime));
		Bytes.Append(fUInt32ToBytes(bodyLength));

		// Body
		Bytes.Append(cnBytes);

		resp = ClientSocket->Send(Bytes.GetData(), Bytes.Num(), BytesSent);
	}
}

TArray<uint8> UET_Tcp_Lscs_Motion::fInt32ToBytes(int32 InInt32)
{
	TArray<uint8> ResultBytes;
	InInt32 = fInt32HtoN(InInt32);

	ResultBytes.Append((uint8*)&InInt32, 4);
	return ResultBytes;
}


uint64 UET_Tcp_Lscs_Motion::ReadRequestSequence(const TArray<uint8>& bytes)
{
	
	/*uint64 result = (uint64)( bytes.GetData()[8]  << 56	| bytes.GetData()[9]  << 48
							| bytes.GetData()[10] << 40 | bytes.GetData()[11] << 32
							| bytes.GetData()[12] << 24 | bytes.GetData()[13] << 16 
							| bytes.GetData()[14] << 8	| bytes.GetData()[15]);*/
	uint64 result = 0;
	for (int i = 15; 7 < i; i--) 
	{
		result |= (static_cast<uint64>(bytes.GetData()[i]) << (8 * (15 - i)));
	}
	return result;
}

TArray<uint8> UET_Tcp_Lscs_Motion::fStringToBytes(FString InString)
{
	TArray<uint8> ResultBytes;
	ResultBytes.Append((uint8*)TCHAR_TO_UTF8(*InString), InString.Len());
	return ResultBytes;
}

TArray<uint8> UET_Tcp_Lscs_Motion::fUInt64ToBytes(uint64 InInt64)
{
	TArray<uint8> ResultBytes;
	ResultBytes.Append((uint8*)&InInt64, 8);

	uint8 temp = ResultBytes.GetData()[7];
	ResultBytes.GetData()[7] = ResultBytes.GetData()[0];
	ResultBytes.GetData()[0] = temp;
	temp = ResultBytes.GetData()[6];
	ResultBytes.GetData()[6] = ResultBytes.GetData()[1];
	ResultBytes.GetData()[1] = temp;
	temp = ResultBytes.GetData()[5];
	ResultBytes.GetData()[5] = ResultBytes.GetData()[2];
	ResultBytes.GetData()[2] = temp;
	temp = ResultBytes.GetData()[4];
	ResultBytes.GetData()[4] = ResultBytes.GetData()[3];
	ResultBytes.GetData()[3] = temp;

	return ResultBytes;
}

TArray<uint8> UET_Tcp_Lscs_Motion::fUInt32ToBytes(uint32 InInt32)
{
	TArray<uint8> ResultBytes;
	InInt32 = fUInt32HtoN(InInt32);

	ResultBytes.Append((uint8*)&InInt32, 4);
	return ResultBytes;
}


int32 UET_Tcp_Lscs_Motion::fInt32HtoN(int32 fVal)
{
	int32 returnVal;
	TArray<uint8> param;
	param.Append((uint8*)&fVal, 4);

	returnVal = param[0] << 24 | param[1] << 16 | param[2] << 8 | param[3];

	return returnVal;
}

uint32 UET_Tcp_Lscs_Motion::fUInt32HtoN(uint32 fVal)
{
	uint32 returnVal;
	TArray<uint8> param;
	param.Append((uint8*)&fVal, 4);

	returnVal = param[0] << 24 | param[1] << 16 | param[2] << 8 | param[3];
	return returnVal;
}