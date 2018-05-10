.model flat, stdcall

.code

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Vec3PointTransformSSE proc pRes : DWORD, pData : DWORD, m : DWORD

	mov eax, pData
	movss xmm0, dword ptr [eax]
	movss xmm1, dword ptr [eax+4]
	movss xmm2, dword ptr [eax+8]

	mov eax, m
	shufps xmm0, xmm0, 0
	shufps xmm1, xmm1, 0
	shufps xmm2, xmm2, 0
	mulps xmm0, [eax]
	mulps xmm1, [eax+10h]
	mulps xmm2, [eax+20h]
	addps xmm1, xmm0
	addps xmm2, [eax+30h]
	addps xmm2, xmm1

	mov eax, pRes
	movlps qword ptr [eax], xmm2
	movhlps xmm2, xmm2
	movss dword ptr [eax+8], xmm2

	ret

Vec3PointTransformSSE endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Vec3PointTransformArraySSE proc	pRes : DWORD, pData : DWORD, m : DWORD, count : DWORD

	movdqu oword ptr[esp-10h], xmm6

	mov eax, m
	movaps xmm3, [eax]
	movaps xmm4, [eax+10h]
	movaps xmm5, [eax+20h]
	movaps xmm6, [eax+30h]

	mov edx, pData
	mov ecx, pRes
	mov eax, count

mulLoop:

	movss xmm0, dword ptr [edx]
	movss xmm1, dword ptr [edx+4]
	movss xmm2, dword ptr [edx+8]
	add edx, 12
	prefetchnta [edx]

	shufps xmm0, xmm0, 0
	shufps xmm1, xmm1, 0
	shufps xmm2, xmm2, 0
	mulps xmm0, xmm3
	mulps xmm1, xmm4
	mulps xmm2, xmm5
	addps xmm1, xmm0
	addps xmm2, xmm6
	addps xmm2, xmm1

	movlps qword ptr [ecx], xmm2
	movhlps xmm2, xmm2
	movss dword ptr [ecx+8], xmm2
	add ecx, 12
	prefetchnta [ecx]

	sub eax, 1
	jnz mulLoop

	movdqu xmm6, oword ptr [esp-10h]

	ret

Vec3PointTransformArraySSE endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Vec4PointTransformArraySSE proc pRes : DWORD, pData : DWORD, m : DWORD, count : DWORD

	movdqu oword ptr [esp-10h], xmm6
	movdqu oword ptr [esp-20h], xmm7

	mov eax, m
	movaps xmm4, [eax]
	movaps xmm5, [eax+10h]
	movaps xmm6, [eax+20h]
	movaps xmm7, [eax+30h]

	mov edx, pData
	mov ecx, pRes
	mov eax, count

mulLoop:

	movaps xmm0, [edx]
	add edx, 16
	prefetchnta oword ptr [edx]

	movaps xmm1, xmm0
	movaps xmm2, xmm0
	movaps xmm3, xmm0
	shufps xmm0, xmm0, 0
	shufps xmm1, xmm1, 055h
	shufps xmm2, xmm2, 0AAh
	shufps xmm3, xmm3, 0FFh
	mulps xmm0, xmm4
	mulps xmm1, xmm5
	mulps xmm2, xmm6
	mulps xmm3, xmm7
	addps xmm1, xmm0
	addps xmm3, xmm2
	addps xmm3, xmm1
	
	movaps [ecx], xmm3
	add ecx, 16
	prefetchnta oword ptr [ecx]

	sub eax, 1
	jnz mulLoop

	movdqu xmm7, oword ptr [esp-20h]
	movdqu xmm6, oword ptr [esp-10h]

	ret

Vec4PointTransformArraySSE endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Vec3NormalTransformSSE proc pRes : DWORD, pData : DWORD, m : DWORD

	mov eax, pData
	movss xmm0, dword ptr [eax]
	movss xmm1, dword ptr [eax+4]
	movss xmm2, dword ptr [eax+8]

	mov eax, m
	movaps xmm3, [eax]
	movaps xmm4, [eax+10h]
	movaps xmm5, [eax+20h]

	shufps xmm0, xmm0, 0
	shufps xmm1, xmm1, 0
	shufps xmm2, xmm2, 0
	mulps xmm0, xmm3
	mulps xmm1, xmm4
	mulps xmm2, xmm5
	addps xmm1, xmm0
	addps xmm2, xmm1

	mov eax, pRes
	movlps qword ptr [eax], xmm2
	movhlps xmm2, xmm2
	movss dword ptr [eax+8], xmm2

	ret

Vec3NormalTransformSSE endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Vec3NormalTransformArraySSE proc pRes : DWORD, pData : DWORD, m : DWORD, count : DWORD

	mov eax, m
	movaps xmm3, [eax]
	movaps xmm4, [eax+10h]
	movaps xmm5, [eax+20h]

	mov edx, pData
	mov ecx, pRes
	mov eax, count

mulLoop:

	movss xmm0, dword ptr [edx]
	movss xmm1, dword ptr [edx+4]
	movss xmm2, dword ptr [edx+8]
	add edx, 12
	prefetchnta oword ptr [edx]

	shufps xmm0, xmm0, 0
	shufps xmm1, xmm1, 0
	shufps xmm2, xmm2, 0
	mulps xmm0, xmm3
	mulps xmm1, xmm4
	mulps xmm2, xmm5
	addps xmm1, xmm0
	addps xmm2, xmm1

	movlps qword ptr [ecx], xmm2
	movhlps xmm2, xmm2
	movss dword ptr [ecx+8], xmm2
	add ecx, 12
	prefetchnta oword ptr [ecx]

	sub eax, 1
	jnz mulLoop

	ret

Vec3NormalTransformArraySSE endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

end