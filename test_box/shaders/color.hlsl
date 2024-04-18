//***************************************************************************************
// color.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************
cbuffer cb_per_object : register(b0)
{
	float4x4 g_world_view_proj; 
};
//
struct vertex_in
{
	float3 pos_l : POSITION;
	float4 color : COLOR;
};
//
struct vertex_out
{
	float4 pos_h : SV_POSITION;
	float4 color : COLOR;
};
//
vertex_out VS(vertex_in vin)
{
	vertex_out vout;
	// Transform to homogeneous clip space.
	vout.pos_h = mul(float4(vin.pos_l, 1.0f), g_world_view_proj);
	// Just pass vertex color into the pixel shader.
	vout.color = vin.color;
	return vout;
}
//
float4 PS(vertex_out pin) : SV_Target
{
	return pin.color;
}
