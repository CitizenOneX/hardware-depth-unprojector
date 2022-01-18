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

Shader "Custom/VertexColorSize"
{
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        Pass
        {
            CGPROGRAM

            #pragma vertex vert
            #pragma fragment frag

            #include "UnityCG.cginc"
 
            struct VertexData
            {
                float4 position : POSITION;
                float4 color : COLOR;
            };

            struct VertexOutput
            {
                float4 position : SV_POSITION;
                float4 color : COLOR;
                float size : PSIZE;
            };

            float4x4 transform; //typically local to world matrix

            StructuredBuffer<VertexData> vertices;

            VertexOutput vert(uint vid : SV_VertexID)
            {
                VertexData vin = vertices[vid];
                VertexOutput vout;

                vout.position =  mul(transform, vin.position);
                vout.position = UnityObjectToClipPos(vout.position);

                vout.color = vin.color;
                vout.size = 4.0; //disable size computation for now

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
