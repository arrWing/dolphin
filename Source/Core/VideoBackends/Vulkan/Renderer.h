// Copyright 2016 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <tuple>

#include "Common/CommonTypes.h"
#include "VideoBackends/Vulkan/Constants.h"
#include "VideoCommon/RenderBase.h"

namespace Vulkan
{
class BoundingBox;
class FramebufferManager;
class SwapChain;
class Texture2D;
class RasterFont;
class VKFramebuffer;
class VKPipeline;
class VKTexture;

class Renderer : public ::Renderer
{
public:
  Renderer(std::unique_ptr<SwapChain> swap_chain);
  ~Renderer() override;

  static Renderer* GetInstance();

  bool IsHeadless() const override;

  bool Initialize() override;
  void Shutdown() override;

  std::unique_ptr<AbstractTexture> CreateTexture(const TextureConfig& config) override;
  std::unique_ptr<AbstractStagingTexture>
  CreateStagingTexture(StagingTextureType type, const TextureConfig& config) override;
  std::unique_ptr<AbstractFramebuffer>
  CreateFramebuffer(const AbstractTexture* color_attachment,
                    const AbstractTexture* depth_attachment) override;

  std::unique_ptr<AbstractShader> CreateShaderFromSource(ShaderStage stage, const char* source,
                                                         size_t length) override;
  std::unique_ptr<AbstractShader> CreateShaderFromBinary(ShaderStage stage, const void* data,
                                                         size_t length) override;
  std::unique_ptr<AbstractPipeline> CreatePipeline(const AbstractPipelineConfig& config) override;

  BoundingBox* GetBoundingBox() const { return m_bounding_box.get(); }
  void RenderText(const std::string& pstr, int left, int top, u32 color) override;
  u32 AccessEFB(EFBAccessType type, u32 x, u32 y, u32 poke_data) override;
  void PokeEFB(EFBAccessType type, const EfbPokeData* points, size_t num_points) override;
  u16 BBoxRead(int index) override;
  void BBoxWrite(int index, u16 value) override;
  TargetRectangle ConvertEFBRectangle(const EFBRectangle& rc) override;

  void SwapImpl(AbstractTexture* texture, const EFBRectangle& rc, u64 ticks) override;
  void Flush() override;

  void ClearScreen(const EFBRectangle& rc, bool color_enable, bool alpha_enable, bool z_enable,
                   u32 color, u32 z) override;

  void ReinterpretPixelData(unsigned int convtype) override;

  void ApplyState() override;

  void ResetAPIState() override;
  void RestoreAPIState() override;

  void SetPipeline(const AbstractPipeline* pipeline) override;

  void SetScissorRect(const MathUtil::Rectangle<int>& rc) override;
  void SetTexture(u32 index, const AbstractTexture* texture) override;
  void SetSamplerState(u32 index, const SamplerState& state) override;
  void UnbindTexture(const AbstractTexture* texture) override;
  void SetInterlacingMode() override;
  void SetViewport(float x, float y, float width, float height, float near_depth,
                   float far_depth) override;
  void Draw(u32 base_vertex, u32 num_vertices) override;
  void DrawIndexed(u32 base_index, u32 num_indices, u32 base_vertex) override;

private:
  void BeginFrame();

  void CheckForSurfaceChange();
  void CheckForSurfaceResize();
  void CheckForConfigChanges();

  void ResetSamplerStates();

  void OnSwapChainResized();
  void BindEFBToStateTracker();
  void RecreateEFBFramebuffer();

  void RecompileShaders();
  bool CompileShaders();
  void DestroyShaders();

  // Draw the frame, as well as the OSD to the swap chain.
  void DrawScreen(VKTexture* xfb_texture, const EFBRectangle& xfb_region);

  // Copies/scales an image to the currently-bound framebuffer.
  void BlitScreen(VkRenderPass render_pass, const TargetRectangle& dst_rect,
                  const TargetRectangle& src_rect, const Texture2D* src_tex);

  std::unique_ptr<SwapChain> m_swap_chain;
  std::unique_ptr<BoundingBox> m_bounding_box;
  std::unique_ptr<RasterFont> m_raster_font;

  // Keep a copy of sampler states to avoid cache lookups every draw
  std::array<SamplerState, NUM_PIXEL_SHADER_SAMPLERS> m_sampler_states = {};

  // Shaders used for clear/blit.
  VkShaderModule m_clear_fragment_shader = VK_NULL_HANDLE;
};
}
