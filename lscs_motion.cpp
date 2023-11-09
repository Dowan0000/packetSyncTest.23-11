//ClientConnectionFinishedFuture = Async(EAsyncExecution::Thread, [&]()
//	{
//		uint32 BufferSize = 0;
//		TArray<uint8> ReceiveBuffer;
//		ReceiveBuffer.SetNumUninitialized(4096);
//
//		FString ResultString;
//
//		uint16 DataSize;
//		uint16 rest;
//
//		while (bShouldReceiveData)
//		{
//			if (ClientSocket->HasPendingData(BufferSize))
//			{
//
//				int32 Read = 0;
//				UE_LOG(LogTemp, Warning, TEXT("Before Receive Motion... ... ..."));
//				ClientSocket->Recv(&ReceiveBuffer.GetData()[rest], ReceiveBuffer.Num(), Read);
//				UE_LOG(LogTemp, Warning, TEXT("After Receive Motion... ... ..."));
//
//				DataSize = Read + rest;
//
//				if (DataSize < 34)
//				{
//					rest = DataSize;
//					continue;
//				}
//
//				int32 messageLength = (int32)(ReadBodylength(ReceiveBuffer) + 34);  //messageLength = header(34 bytes) + body
//				//OnReceivedBytes.Broadcast(ReceiveBuffer);
//
//				while (messageLength <= DataSize && messageLength > 0)
//				{
//					UE_LOG(LogTemp, Warning, TEXT("Before BroadCast Motion... ... ..."));
//					OnReceivedBytes.Broadcast(ReceiveBuffer);
//					UE_LOG(LogTemp, Warning, TEXT("After BroadCast Motion... ... ..."));
//					//count++;
//
//					DataSize -= messageLength;
//
//					//UE_LOG(LogTemp, Error, TEXT("Receiving Motion ... ... ... %d"), count);
//					if (messageLength >= DataSize) break;
//
//					// *****
//					for (int i = 0; i < messageLength; i++)
//						ReceiveBuffer.RemoveAt(0);
//
//
//					messageLength = (int32)(ReadBodylength(ReceiveBuffer) + 34);  //messageLength = header(34 bytes) + body
//
//					//FPlatformProcess::Sleep(.01f);
//				}
//
//				rest = DataSize;
//			}
//
//			ClientSocket->Wait(ESocketWaitConditions::WaitForReadOrWrite, FTimespan(1));
//		}
//	});