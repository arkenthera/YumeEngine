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
// File : YumeGraphics.h
// Date : 2.19.2016
// Comments :
//
//----------------------------------------------------------------------------
#ifndef __YumeD3D11Graphics_h__
#define __YumeD3D11Graphics_h__
//----------------------------------------------------------------------------
#include "YumeD3D11Required.h"
#include "Renderer/YumeRendererDefs.h"

#include "Math/YumeVector2.h"
#include "Math/YumeVector4.h"

#include "Renderer/YumeRenderer.h"

#include <boost/thread/mutex.hpp>
//----------------------------------------------------------------------------
namespace YumeEngine
{
	class YumeGpuResource;
	class YumeD3D11RendererImpl;

	typedef std::vector<YumeGpuResource*> GpuResourceVector;
	class YumeD3DExport YumeD3D11Renderer : public YumeRenderer
	{
	public:
		YumeD3D11Renderer();
		~YumeD3D11Renderer();


		bool SetGraphicsMode(int width,int height,bool fullscreen,bool borderless,bool resizable,bool vsync,bool tripleBuffer,
			int multiSample);

		bool BeginFrame();
		void EndFrame();
		void Clear(unsigned flags, const Vector4& color = Vector4(0.0f, 1.0f, 0.0f, 0.0f), float depth = 1.0f, unsigned stencil = 0);

		bool CreateD3D11Device(int width,int height,int multisample);
		bool UpdateSwapchain(int width,int height);

		void ResetRenderTargets();
		void SetViewport(const Vector4&);

		Vector2 GetRenderTargetDimensions() const;

		void SetFlushGPU(bool flushGpu);
		void CreateRendererCapabilities();
		YumeVector<int>::type GetMultiSampleLevels() const;

		//Window ops
		bool OpenWindow(int width,int height,bool resizable,bool borderless);
		void Close();
		void AdjustWindow(int& newWidth,int& newHeight,bool& newFullscreen,bool& newBorderless);
		void Maximize();
		void SetWindowPos(const Vector2& pos);
		void SetWindowTitle(const YumeString& string);

		void AddGpuResource(YumeGpuResource* object);
		void RemoveGpuResource(YumeGpuResource* object);


		bool IsInitialized() const { return initialized_; }

		YumeD3D11RendererImpl* GetImpl() const { return impl_; }
	private:
		YumeVector<Vector2>::type				GetScreenResolutions();
	private:
		bool									initialized_;

		YumeString								windowTitle_;
		int										windowWidth_;
		int										windowHeight_;
		Vector2									windowPos_;
		bool									fullscreen_;
		bool									borderless_;
		bool									resizeable_;
		bool									tripleBuffer_;
		int										multiSample_;
		bool									vsync_;
		bool									flushGpu_;

		int										numPrimitives_;
		int										numBatches_;

		Vector4									viewport_;

		//Renderer features
		bool sRGB_;
		bool lightPrepassSupport_;
		bool deferredSupport_;
		bool hardwareShadowSupport_;
		bool instancingSupport_;
		bool sRGBSupport_;
		bool sRGBWriteSupport_;
		unsigned dummyColorFormat_;
		unsigned shadowMapFormat_;
		unsigned hiresShadowMapFormat_;

		bool renderTargetsDirty_;
		bool texturesDirty_;
		bool vertexDeclarationDirty_;
		bool blendStateDirty_;
		bool depthStateDirty_;
		bool rasterizerStateDirty_;
		bool scissorRectDirty_;
		bool stencilRefDirty_;


		boost::mutex							gpuResourceMutex_;
		GpuResourceVector						gpuResources_;

		YumeD3D11RendererImpl*						impl_;
	};
}


//----------------------------------------------------------------------------
#endif