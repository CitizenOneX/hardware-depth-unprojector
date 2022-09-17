/*
 * Hardware Depth Unprojector (shader)
 * 
 * Copyright 2020 (C) Bartosz Meglicki <meglickib@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Shader for rendering point cloud from StructuredBuffer.
 * Includes colors and varies size with distance from the camera.
 *
 * This is Unity flavour HLSL. You may recompile it with Unity for OpenGL, Vulkan etc.
 */

Shader "Custom/VertexColorMesh"
{
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        Pass
        {
            CGPROGRAM

            #pragma vertex vert
            #pragma fragment frag

            // include debug symbols for RenderDoc
            //#pragma enable_d3d11_debug_symbols

            #include "UnityCG.cginc"

            struct VertexData
            {
                float4 position : POSITION;
                uint2 uv : TEXCOORD0; // actual pixel coords, not [0..1]
            };

            struct VertexOutput
            {
                float4 position : SV_POSITION;
                float4 color : COLOR;
            };

            float4x4 transform; //typically local to world matrix

            StructuredBuffer<VertexData> vertices;
            Texture2D<float> colorTextureY; //Y plane of color corresponding to depth sample
            Texture2D<float2> colorTextureUV; //UV plane of color corresponding to depth sample (width/2, height/2) floats

            VertexOutput vert(uint vid : SV_VertexID)
            {
                VertexData vin = vertices[vid];
                VertexOutput vout;

                vout.position = UnityObjectToClipPos(mul(transform, vin.position));

                float y1 = colorTextureY[vin.uv];
                float2 uv1 = colorTextureUV[vin.uv/2] - 0.5;
                vout.color = float4( 
                    y1 + 1.370705 * (uv1.y), 
                    y1 - 0.698001 * (uv1.y) - (0.337633 * (uv1.x)), 
                    y1 + 1.732446 * (uv1.x), 
                    1);

                return vout;
            }

            float4 frag(VertexOutput output) : SV_Target
            {
                return output.color;
            }

            ENDCG
        }
    }
}
