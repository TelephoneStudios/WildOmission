// Copyright Telephone Studios. All Rights Reserved.


#include "UI/WorkshopItemWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "HttpModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Log.h"

UWorkshopItemWidget::UWorkshopItemWidget(const FObjectInitializer& ObjectInitializer) : UUserWidget(ObjectInitializer)
{
	SetIsFocusable(true);

	Button = nullptr;
	NameTextBlock = nullptr;
	PreviewImage = nullptr;
	
}
void UWorkshopItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Button->OnClicked.AddDynamic(this, &UWorkshopItemWidget::OnButtonClicked);
}

void UWorkshopItemWidget::Setup(const FSteamWorkshopItemDetails& InDetails)
{
	ItemDetails = InDetails;
	NameTextBlock->SetText(FText::FromString(InDetails.Title));
	
	DownloadPreviewTexture(InDetails.PreviewURL);
	//PreviewImage->SetBrushFromTexture(Details.PreviewTexture);

	// todo set preview image
	// todo set identification
}

void UWorkshopItemWidget::NativeTick(const FGeometry& MyGeomotry, float InDeltaTime)
{
	Super::NativeTick(MyGeomotry, InDeltaTime);

}

void UWorkshopItemWidget::DownloadPreviewTexture(const FString& URL)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(URL);
	Request->SetVerb(TEXT("GET"));

	// Use a lambda binder instead of BindUObject to cleanly pass the integer index through the HTTP event
	Request->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
			if (!bWasSuccessful || !Response.IsValid() || Response->GetContentLength() <= 0) return;

			// 1. Get the raw binary payload array
			const TArray<uint8>& RawImageData = Response->GetContent();

			// 2. Detect image format automatically (PNG, JPEG, etc.) via ImageWrapper
			IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
			EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(RawImageData.GetData(), RawImageData.Num());

			if (ImageFormat == EImageFormat::Invalid) return;

			TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
			if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(RawImageData.GetData(), RawImageData.Num())) return;

			// 3. Decompress the image into uncompressed raw RGBA byte data
			TArray<uint8> UncompressedBGRA;
			if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA)) return;

			// 4. Construct a new Transient Texture on the GameThread safely
			int32 Width = ImageWrapper->GetWidth();
			int32 Height = ImageWrapper->GetHeight();

			UTexture2D* LoadedTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
			if (!LoadedTexture) return;

			// 5. Bulk copy the decoded pixel array directly into the texture's platform memory
			void* TextureData = LoadedTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
			LoadedTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

			// Update the texture properties so the GPU renders it correctly
			LoadedTexture->UpdateResource();

			ItemDetails.PreviewTexture = LoadedTexture;
			
			PreviewImage->SetBrushFromTexture(LoadedTexture);
		});

	Request->ProcessRequest();
}

void UWorkshopItemWidget::OnPreviewDownloaded(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid() || Response->GetContentLength() <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to download workshop preview image."));
		return;
	}

	// 1. Get the raw binary payload array
	const TArray<uint8>& RawImageData = Response->GetContent();

	// 2. Detect image format automatically (PNG, JPEG, etc.) via ImageWrapper
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(RawImageData.GetData(), RawImageData.Num());

	if (ImageFormat == EImageFormat::Invalid) return;

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);
	if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(RawImageData.GetData(), RawImageData.Num())) return;

	// 3. Decompress the image into uncompressed raw RGBA byte data
	TArray<uint8> UncompressedBGRA;
	if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA)) return;

	// 4. Construct a new Transient Texture on the GameThread safely
	int32 Width = ImageWrapper->GetWidth();
	int32 Height = ImageWrapper->GetHeight();

	UTexture2D* LoadedTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!LoadedTexture) return;

	// 5. Bulk copy the decoded pixel array directly into the texture's platform memory
	void* TextureData = LoadedTexture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
	LoadedTexture->GetPlatformData()->Mips[0].BulkData.Unlock();

	// Update the texture properties so the GPU renders it correctly
	LoadedTexture->UpdateResource();

	// 6. Broadcast your texture out to UI or Materials!
	// OnPreviewTextureReady.Broadcast(LoadedTexture);
}

void UWorkshopItemWidget::OnButtonClicked()
{
	if (OnClicked.IsBound())
	{
		OnClicked.Broadcast(ItemDetails);
	}
}