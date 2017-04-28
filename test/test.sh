# simple test case script by Radek Vít
#test folder
tf=tests
sretval=0

# TestCaseArgs()
# $0: function name
# $1: test name
# $2: other arguments
TestCaseArgs() {
	echo "$1 argument files"
	if .././reon -i $tf/$1_in -o $tf/$1_out $2 && diff $tf/$1_expected $tf/$1_out ; then
		echo "success"
		retval=0
		rm $tf/$1_out	
	else
		echo "FAILED"
		retval=1
	fi

	return $retval
}

# TestCaseRedirection()
# $0: function name
# $1: test name
# $2: other arguments
TestCaseRedirection() {
	echo "$1 i/o redirection"
	if .././reon $2 < $tf/$1_in > $tf/$1_out && diff $tf/$1_expected $tf/$1_out ; then
		echo "success"
		retval=0
		rm $tf/$1_out		
	else
		echo "FAILED"
		retval=1
	fi

	return $retval
}

#require that reon throws an error code
# $0: function name
# $1: fail test name
# $2: expected return code
# $3: other arguments
TestFail() {
	echo "expecting $1 to fail"
	.././reon $3 < $tf/$1_in >> /dev/null 2>> /dev/null
	ret=$?
	if [ $ret -ne 0 ]; then
		if [ $ret -eq $2 ] ; then
			echo "failed successfully"
		else
			echo "failed with error code $ret instead of $2"
		fi
	else
		echo "ERROR: did not fail"
	fi
}

#success tests
i=1
testcount=`ls $tf/test*_in | wc -l`
while [ $i -le $testcount ]; do
	# argument file for the test
	touch $tf/test${i}_arg
	TestCaseArgs test$i "`cat $tf/test${i}_arg`" -ne 0
	if [ $? -ne 0 ] ; then
		sretval=1
	fi
	TestCaseRedirection test$i "`cat $tf/test${i}_arg`"
	if [ $? -ne 0 ] ; then
		sretval=1
	fi
	i=$(( i + 1))
done

#fail tests
i=1
testcount=`ls $tf/fail*_in | wc -l`
while [ $i -le $testcount ]; do
	# argument file for the test
	touch $tf/fail${i}_arg
	touch $tf/fail${i}_retcode
	TestFail fail$i `cat $tf/fail${i}_retval` "`cat $tf/fail${i}_arg`"
	if [ $? -ne 0 ] ; then
		sretval=1
	fi
	i=$(( i + 1))
done

if [ $retval -ne 0 ] ; then
	echo "Tests failed."
fi
return $sretval
