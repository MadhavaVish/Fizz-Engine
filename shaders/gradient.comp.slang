[[vk::binding(0, 0)]]
uniform RWTexture2D<float4> drawImage;

[shader("compute")]
[numthreads(16, 16, 1)]
void main(uint3 threadId: SV_DispatchThreadID)
{
    uint2 index = threadId.xy;
    uint2 size;
    uint numlevels;
    drawImage.GetDimensions(0, size.x, size.y, numlevels);
    if (any(threadId.xy >= size))
    {
        return;
    }
    float4 color = float4(0.0, 0.0, 0.0, 1.0);
    if (index.x % 20 == 0 || index.y % 20 == 0)
    {
        color = float4(1.0, 1.0, 1.0, 1.0);
        printf("balls");
    }
    drawImage[index] = color;
}
