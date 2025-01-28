for ex in {6..9}
do
	for cors in {1..8}
	do
		let n=10**$ex
		./out $n $cors
	done
done
