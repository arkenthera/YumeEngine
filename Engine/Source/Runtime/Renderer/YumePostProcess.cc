//----------------------------------------------------------------------------
//Yume Engine
//Copyright (C) 2015  arkenthera
//This program is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//You should have received a copy of the GNU General Public License along
//with this program; if not, write to the Free Software Foundation, Inc.,
//51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.*/
//----------------------------------------------------------------------------
//
// File : <Filename>
// Date : <Date>
// Comments :
//
//----------------------------------------------------------------------------
#include "YumeHeaders.h"
#include "YumePostProcess.h"

#include "YumeRHI.h"
#include "YumeMiscRenderer.h"

#include "LPVRendererTest.h"

#include "YumeTexture3D.h"


namespace YumeEngine
{
	YumePostProcess::YumePostProcess(YumeMiscRenderer* misc)
		: ssao_enabled(false),
		ssao_scale(0.1),
		godrays_enabled(false),
		godrays_tau(0.0005f),
		dof_enabled(false),
		dof_focal_plane(2000),
		dof_coc_scale(0.8f),
		bloom_enabled(false),
		bloom_sigma(0.5f),
		bloom_treshold(0.5f),
		fxaa_enabled(false),
		exposure_adapt(false),
		exposure_key(0.05),
		exposure_speed(0.4),
		misc_(misc)
	{
	}

	YumePostProcess::~YumePostProcess()
	{

	}

	void YumePostProcess::Setup()
	{
		temporaryRt_ = gYume->pRHI->CreateTexture2D();
		temporaryRt_->SetSize(gYume->pRHI->GetWidth(),gYume->pRHI->GetHeight(),gYume->pRHI->GetRGBAFloat16FormatNs(),TEXTURE_RENDERTARGET,1,10);

		temporaryRt_ = gYume->pRHI->CreateTexture2D();
		temporaryRt_->SetSize(gYume->pRHI->GetWidth(),gYume->pRHI->GetHeight(),gYume->pRHI->GetRGBAFloat16FormatNs(),TEXTURE_RENDERTARGET,1,10);

		frontBufferBlurred_ = gYume->pRHI->CreateTexture2D();
		frontBufferBlurred_->SetSize(gYume->pRHI->GetWidth(),gYume->pRHI->GetHeight(),gYume->pRHI->GetRGBAFloat16FormatNs(),TEXTURE_RENDERTARGET,1,10);


		adaptadLuminanceRt_[0] = gYume->pRHI->CreateTexture2D();
		adaptadLuminanceRt_[0]->SetSize(1,1,gYume->pRHI->GetFloat32FormatNs(),TEXTURE_RENDERTARGET);

		adaptadLuminanceRt_[1] = gYume->pRHI->CreateTexture2D();
		adaptadLuminanceRt_[1]->SetSize(1,1,gYume->pRHI->GetFloat32FormatNs(),TEXTURE_RENDERTARGET);

		bloomFull_ = gYume->pRHI->CreateTexture2D();
		bloomFull_->SetSize(gYume->pRHI->GetWidth(),gYume->pRHI->GetHeight(),gYume->pRHI->GetRGBAFloat16FormatNs(),TEXTURE_RENDERTARGET,1,10);


		blurred_[0] = gYume->pRHI->CreateTexture2D();
		blurred_[0]->SetSize(gYume->pRHI->GetWidth() / 2,gYume->pRHI->GetHeight() / 2,gYume->pRHI->GetRGBAFloat16FormatNs(),TEXTURE_RENDERTARGET,1,0);

		blurred_[5] = gYume->pRHI->CreateTexture2D();
		blurred_[5]->SetSize(gYume->pRHI->GetWidth() / 2,gYume->pRHI->GetHeight() / 2,gYume->pRHI->GetRGBAFloat16FormatNs(),TEXTURE_RENDERTARGET,1,0);

		blurred_[1] = gYume->pRHI->CreateTexture2D();
		blurred_[1]->SetSize(gYume->pRHI->GetWidth() /4,gYume->pRHI->GetHeight() / 4,gYume->pRHI->GetRGBAFloat16FormatNs(),TEXTURE_RENDERTARGET,1,0);

		blurred_[2] = gYume->pRHI->CreateTexture2D();
		blurred_[2]->SetSize(gYume->pRHI->GetWidth() /8,gYume->pRHI->GetHeight() / 8,gYume->pRHI->GetRGBAFloat16FormatNs(),TEXTURE_RENDERTARGET,1,0);

		blurred_[3] = gYume->pRHI->CreateTexture2D();
		blurred_[3]->SetSize(gYume->pRHI->GetWidth() /16,gYume->pRHI->GetHeight() / 16,gYume->pRHI->GetRGBAFloat16FormatNs(),TEXTURE_RENDERTARGET,1,0);

		blurred_[4] = gYume->pRHI->CreateTexture2D();
		blurred_[4]->SetSize(gYume->pRHI->GetWidth() /16,gYume->pRHI->GetHeight() / 16,gYume->pRHI->GetRGBAFloat16FormatNs(),TEXTURE_RENDERTARGET,1,0);

		ssaoPs_ = gYume->pRHI->GetShader(PS,"PostFX/pp_ssao");
		adaptExposure_ = gYume->pRHI->GetShader(PS,"PostFX/pp_bloom","","ps_adapt_exposure");
		bloomThreshold_ = gYume->pRHI->GetShader(PS,"PostFX/pp_bloom","BLOOM_THRESHOLD","ps_bloom_treshold");
		bloom_ = gYume->pRHI->GetShader(PS,"PostFX/pp_bloom","BLOOM_FINAL","ps_bloom");

		godrays_ = gYume->pRHI->GetShader(PS,"PostFX/pp_godrays","GODRAYS_HALFRES","ps_godrays_halfres");
		godraysMerge_ = gYume->pRHI->GetShader(PS,"PostFX/pp_godrays","GODRAYS_MERGE","ps_godrays_merge");

		dof_ = gYume->pRHI->GetShader(PS,"PostFX/pp_dof","","ps_depth_of_field");

		fxaa_ = gYume->pRHI->GetShader(PS,"PostFX/pp_fxaa","","ps_fxaa");

		gaussBloomV_ = gYume->pRHI->GetShader(PS,"PostFX/pp_blur","GAUSS_BLOOM_V","ps_gauss_bloom_v");
		gaussBloomH_ = gYume->pRHI->GetShader(PS,"PostFX/pp_blur","GAUSS_BLOOM_H","ps_gauss_bloom_h");

		gaussDofV_ = gYume->pRHI->GetShader(PS,"PostFX/pp_blur","GAUSS_DOF_V","ps_gauss_dof_v");
		gaussDofH_ = gYume->pRHI->GetShader(PS,"PostFX/pp_blur","GAUSS_DOF_H","ps_gauss_dof_h");

		gaussGodraysV_ = gYume->pRHI->GetShader(PS,"PostFX/pp_blur","GAUSS_GODRAYS_V","ps_gauss_godrays_v");
		gaussGodraysH_ = gYume->pRHI->GetShader(PS,"PostFX/pp_blur","GAUSS_GODRAYS_H","ps_gauss_godrays_h");

		copy_ = gYume->pRHI->GetShader(PS,"PostFX/pp_blur","","ps_copy");

		noiseTex_ = gYume->pResourceManager->PrepareResource<YumeTexture3D>("Textures/noise.dds");
	}

	void YumePostProcess::Render()
	{
		RHIEvent e("Post Processing");

		gYume->pRHI->GenerateMips(misc_->GetRenderTarget());

		static bool current = true;
		current = !current;

		gYume->pRHI->SetViewport(IntRect(0,0,1,1));

		YumeTexture2D* textures[] ={adaptadLuminanceRt_[!current]};
		gYume->pRHI->PSBindSRV(11,1,textures);

		{
			RHIEvent e("Adapt Luminance");
			Render(adaptExposure_,misc_->GetRenderTarget(),adaptadLuminanceRt_[current]);
		}

		if(bloom_enabled)
			Bloom(misc_->GetRenderTarget());


		YumeTexture2D* textures2[] ={adaptadLuminanceRt_[current],bloomFull_};
		gYume->pRHI->PSBindSRV(11,2,textures2);

		SetViewport(misc_->GetRenderTarget());

		YumeTexture2D* in = misc_->GetRenderTarget();
		YumeTexture2D* out = temporaryRt_;

		{
			RHIEvent e("SSAO Pass");
			Render(ssaoPs_,in,out);
			std::swap(in,out);
		}

		{
			DoF(in,out);
			std::swap(in,out);
		}

		{
			Godrays(in,out);
			std::swap(in,out);
		}


		{
			RHIEvent e("Bloom Final");
			Render(bloom_,in,out);
			std::swap(in,out);
		}


		{
			RHIEvent e("FXAA");
			Render(fxaa_,in,0);
			std::swap(in,out);
		}

		gYume->pRHI->BindResetTextures(0,13);
	}

	void YumePostProcess::Bloom(YumeTexture2D* frontbuffer)
	{
		RHIEvent e("Bloom Pass");

		SetViewport(bloomFull_);

		Render(bloomThreshold_,frontbuffer,bloomFull_);
		BloomBlur(bloomFull_,bloomFull_);
	}


	void YumePostProcess::BloomBlur(YumeTexture2D* in,YumeTexture2D* out)
	{
		RHIEvent e("Bloom Blur Pass");

		SetViewport(blurred_[0]);
		Render(copy_,in,blurred_[0]);

		SetViewport(blurred_[1]);
		Render(copy_,blurred_[0],blurred_[1]);

		SetViewport(blurred_[2]);
		Render(copy_,blurred_[1],blurred_[2]);

		SetViewport(blurred_[3]);
		Render(copy_,blurred_[2],blurred_[3]);

		for(int i=0; i < 4; ++i)
		{
			//Vertical Blur
			Render(gaussBloomV_,blurred_[3],blurred_[4]);

			//Horizontal Blur
			Render(gaussBloomH_,blurred_[4],blurred_[3]);
		}

		SetViewport(blurred_[2]);
		Render(copy_,blurred_[3],blurred_[2]);

		SetViewport(blurred_[1]);
		Render(copy_,blurred_[2],blurred_[1]);

		SetViewport(blurred_[0]);
		Render(copy_,blurred_[1],blurred_[0]);

		SetViewport(out);
		Render(copy_,blurred_[0],out);
	}

	void YumePostProcess::DoF(YumeTexture2D* in,YumeTexture2D* out)
	{
		RHIEvent e("DoF");

		if(dof_enabled)
			DoFBlur(in,frontBufferBlurred_);

		YumeTexture2D* textures[] ={frontBufferBlurred_};
		YumeTexture2D* texturesNull[] ={nullptr};

		gYume->pRHI->PSBindSRV(13,1,textures);

		Render(dof_,in,out);

		gYume->pRHI->PSBindSRV(13,1,texturesNull);
	}

	void YumePostProcess::DoFBlur(YumeTexture2D* in,YumeTexture2D* out)
	{
		RHIEvent e("DoF Blur");

		YumeTexture2D* textures[] ={in};

		gYume->pRHI->PSBindSRV(0,1,textures);

		SetViewport(blurred_[0]);
		Render(copy_,in,blurred_[0]);

		for(size_t i = 0; i < 2; ++i)
		{
			// apply v-blur
			Render(gaussDofV_,blurred_[0],blurred_[5]);

			// apply h-blur
			Render(gaussDofH_,blurred_[5],blurred_[0]);
		}

		SetViewport(out);
		Render(copy_,blurred_[0],out);

		YumeTexture2D* texturesNull[] ={nullptr};
		gYume->pRHI->PSBindSRV(0,1,texturesNull);
	}


	void YumePostProcess::Godrays(YumeTexture2D* in,YumeTexture2D* out)
	{
		RHIEvent e("Godrays");

		if(godrays_enabled)
		{
			
			YumeTexture3D* noiseTex[] ={ noiseTex_ };

			gYume->pRHI->PSBindSRV(14,1,noiseTex);
			

			SetViewport(blurred_[0]);
			Render(copy_,in,blurred_[0]);

			Render(godrays_,blurred_[0],blurred_[5]);


			YumeTexture2D* textures[] ={nullptr};
			gYume->pRHI->PSBindSRV(0,1,textures);


			for(size_t i = 0; i < 2; ++i)
			{
				// apply v-blur
				Render(gaussGodraysV_,blurred_[5],blurred_[0]);

				// apply h-blur
				Render(gaussGodraysH_,blurred_[0],blurred_[5]);
			}

			SetViewport(out);
			Render(copy_,blurred_[5],frontBufferBlurred_);
		}

		YumeTexture2D* textures[] ={frontBufferBlurred_};
		YumeTexture2D* texturesNull[] ={nullptr};

		gYume->pRHI->PSBindSRV(13,1,textures);

		Render(godraysMerge_,in,out);

		gYume->pRHI->PSBindSRV(13,1,texturesNull);


	}

	void YumePostProcess::Render(YumeShaderVariation* ps,YumeTexture2D* in,YumeTexture2D* out)
	{
		YumeShaderVariation* triangle = gYume->pRHI->GetShader(VS,"LPV/fs_triangle");


		gYume->pRHI->SetShaders(triangle,ps,0);
		misc_->SetCameraParameters(false);

		YumeGeometry* fs = misc_->GetFsTriangle();

		YumeTexture2D* textures[] ={in};
		gYume->pRHI->PSBindSRV(10,1,textures);

		misc_->GetLPVRenderer()->SetDeferredLightParameters();

		//TODO Move these out of here
		gYume->pRHI->SetShaderParameter("scene_dim_max",DirectX::XMFLOAT4(misc_->GetMaxBb().x,misc_->GetMaxBb().y,misc_->GetMaxBb().z,1.0f));
		gYume->pRHI->SetShaderParameter("scene_dim_min",DirectX::XMFLOAT4(misc_->GetMinBb().x,misc_->GetMinBb().y,misc_->GetMinBb().z,1.0f));

		SetPPParameters();
		misc_->SetPerFrameConstants();



		if(out)
		{
			gYume->pRHI->SetRenderTarget(0,out);
		}
		else
			gYume->pRHI->BindBackbuffer();

		fs->Draw(gYume->pRHI);


		gYume->pRHI->BindResetRenderTargets(1);
		gYume->pRHI->BindResetTextures(10,1);
	}

	void YumePostProcess::SSAO(YumeTexture2D* target)
	{
		RHIEvent e("SSAO Pass");
		Render(ssaoPs_,misc_->GetRenderTarget(),target);
	}

	void YumePostProcess::SetViewport(YumeTexture2D* rendertarget)
	{
		gYume->pRHI->SetViewport(IntRect(0,0,rendertarget->GetWidth(),rendertarget->GetHeight()));
	}

	void YumePostProcess::SetPPParameters()
	{
		gYume->pRHI->SetShaderParameter("ssao_enabled",ssao_enabled);
		gYume->pRHI->SetShaderParameter("ssao_scale",ssao_scale);

		gYume->pRHI->SetShaderParameter("godrays_enabled",godrays_enabled);
		gYume->pRHI->SetShaderParameter("godrays_tau",godrays_tau);

		gYume->pRHI->SetShaderParameter("dof_enabled",dof_enabled);
		gYume->pRHI->SetShaderParameter("dof_focal_plane",dof_focal_plane);
		gYume->pRHI->SetShaderParameter("dof_coc_scale",dof_coc_scale);

		gYume->pRHI->SetShaderParameter("bloom_enabled",bloom_enabled);
		gYume->pRHI->SetShaderParameter("bloom_sigma",bloom_sigma);
		gYume->pRHI->SetShaderParameter("bloom_treshold",bloom_treshold);

		gYume->pRHI->SetShaderParameter("fxaa_enabled",fxaa_enabled);

		gYume->pRHI->SetShaderParameter("exposure_adapt",exposure_adapt);
		gYume->pRHI->SetShaderParameter("exposure_key",exposure_key);
		gYume->pRHI->SetShaderParameter("exposure_speed",exposure_speed);
	}
}
