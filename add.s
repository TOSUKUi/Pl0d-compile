start:
	lit,2     		; 2をスタックに積む
	lit,3     		; 3をスタックに積む
	opr,add   		; 2 + 3の結果をスタックに積む
	opr,wrt   		; スタック上の値を表示
	opr,wrl   		; 改行を表示
	jmp,start 		; 先頭アドレスにジャンプして終了