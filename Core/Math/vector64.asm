.code

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Vec3PointTransformSSE proc		; Vec3& pRes, Vec3& pData, Mat4x4& m

	movss xmm0, dword ptr [rdx]
	movss xmm1, dword ptr [rdx+4]
	movss xmm2, dword ptr [rdx+8]

	shufps xmm0, xmm0, 0
	shufps xmm1, xmm1, 0
	shufps xmm2, xmm2, 0
	mulps xmm0, [r8]
	mulps xmm1, [r8+10h]
	mulps xmm2, [r8+20h]
	addps xmm1, xmm0
	addps xmm2, [r8+30h]
	addps xmm2, xmm1

	movlps qword ptr [rcx], xmm2
	movhlps xmm2, xmm2
	movss dword ptr [rcx+8], xmm2

	ret

Vec3PointTransformSSE endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Vec3PointTransformArraySSE proc		; Vec3* pRes, Vec3* pData, Mat4x4& m, ulong count
	; rdx		pV
	; rcx		pR
	; r9		count

	movdqu oword ptr [rsp+10h], xmm6

	movaps xmm3, [r8]
	movaps xmm4, [r8+10h]
	movaps xmm5, [r8+20h]
	movaps xmm6, [r8+30h]

mulLoop:

	movss xmm0, dword ptr [rdx]
	movss xmm1, dword ptr [rdx+4]
	movss xmm2, dword ptr [rdx+8]
	add rdx, 12
	prefetchnta oword ptr [rdx]

	shufps xmm0, xmm0, 0
	shufps xmm1, xmm1, 0
	shufps xmm2, xmm2, 0
	mulps xmm0, xmm3
	mulps xmm1, xmm4
	mulps xmm2, xmm5
	addps xmm1, xmm0
	addps xmm2, xmm6
	addps xmm2, xmm1

	movlps qword ptr [rcx], xmm2
	movhlps xmm2, xmm2
	movss dword ptr [rcx+8], xmm2
	add rcx, 12
	prefetchnta oword ptr [rcx]

	sub r9, 1
	jnz mulLoop

	movdqu xmm6, oword ptr [rsp+10h]

	ret

Vec3PointTransformArraySSE endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Vec4PointTransformArraySSE proc		; Vec4* pRes, Vec4* pData, Mat4x4& m, ulong count
	; rdx		pData
	; rcx		pRes
	; r9		count

	movdqu oword ptr [rsp+10h], xmm6
	movdqu oword ptr [rsp+20h], xmm7

	movaps xmm4, [r8]
	movaps xmm5, [r8+10h]
	movaps xmm6, [r8+20h]
	movaps xmm7, [r8+30h]

mulLoop:

	movaps xmm0, [rdx]
	add rdx, 16
	prefetchnta oword ptr [rdx]

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
	
	movaps [rcx], xmm3
	add rcx, 16
	prefetchnta oword ptr [rcx]

	sub r9, 1
	jnz mulLoop

	movdqu xmm7, oword ptr [rsp+20h]
	movdqu xmm6, oword ptr [rsp+10h]

	ret

Vec4PointTransformArraySSE endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Vec3NormalTransformSSE proc		; Vec3& pRes, Vec3& pData, Mat4x4& m

	movss xmm0, dword ptr [rdx]
	movss xmm1, dword ptr [rdx+4]
	movss xmm2, dword ptr [rdx+8]

	shufps xmm0, xmm0, 0
	shufps xmm1, xmm1, 0
	shufps xmm2, xmm2, 0
	mulps xmm0, [r8]
	mulps xmm1, [r8+10h]
	mulps xmm2, [r8+20h]
	addps xmm1, xmm0
	addps xmm2, xmm1

	movlps qword ptr [rcx], xmm2
	movhlps xmm2, xmm2
	movss dword ptr [rcx+8], xmm2

	ret

Vec3NormalTransformSSE endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Vec3NormalTransformArraySSE proc		; Vec3* pRes, Vec3* pData, Mat4x4& m, ulong count
; rdx		pV
	; rcx		pR
	; r9		count

	movaps xmm3, [r8]
	movaps xmm4, [r8+10h]
	movaps xmm5, [r8+20h]

mulLoop:	

	movss xmm0, dword ptr [rdx]
	movss xmm1, dword ptr [rdx+4]
	movss xmm2, dword ptr [rdx+8]
	add rdx, 12
	prefetchnta oword ptr [rdx]

	shufps xmm0, xmm0, 0
	shufps xmm1, xmm1, 0
	shufps xmm2, xmm2, 0
	mulps xmm0, xmm3
	mulps xmm1, xmm4
	mulps xmm2, xmm5
	addps xmm1, xmm0
	addps xmm2, xmm1

	movlps qword ptr [rcx], xmm2
	movhlps xmm2, xmm2
	movss dword ptr [rcx+8], xmm2
	add rcx, 12
	prefetchnta oword ptr [rcx]

	sub r9, 1
	jnz mulLoop

	ret

Vec3NormalTransformArraySSE endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

end