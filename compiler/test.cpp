int sum_answer;//0xa060292d
int test_answer[16];

/*****************DATA DEFINE**************/
int v[5];
int w[5];
int dp[6][11];
/******************************************/

/*****************************************
编译原理课程设计任务书【程序实例2】(有修改)
******************************************/
int program(int a, int b, int c)
{
	int i;
	int j;
	i = 0;
	if (a > (b + c))
	{
		j = a + (b * c + 1);
	}
	else
	{
		j = a;
	}
	while (i <= 100)
	{
		i = i + j * 2;
	}
	return i;
}
int demo(int a)
{
	a = a + 2;
	return a * 2;
}
int test0(void)
{
	int a[2][2];
	a[0][0] = 3;
	a[0][1] = a[0][0] + 1;
	a[1][0] = a[0][0] + a[0][1];
	a[1][1] = program(a[0][0], a[0][1], demo(a[1][0]));
	return a[1][1];
}
/*****************************************
计算机系统结构动态流水CPU【程序实例】(有修改)
******************************************/
int test1(void)
{
	int a[60];
	int b[60];
	int c[60];
	int d[60];
	int e[60];
	int i;
	a[0] = 0;
	b[0] = 1;
	c[0] = 0;
	d[0] = 0;
	e[0] = 0;
	i = 1;
	while (i < 60)
	{
		a[i] = a[i - 1] + i;
		b[i] = b[i - 1] + 3 * i;
		if (i < 20)
		{
			c[i] = a[i];
			d[i] = b[i];
		}
		else
		{
			if (i < 40)
			{
				c[i] = a[i] + b[i];
				d[i] = c[i] + a[i];
			}
			else
			{
				c[i] = a[i] * b[i];
				d[i] = c[i] * b[i];
			}
		}
		e[i] = c[i] + d[i];
		i = i + 1;
	}
	return e[59];
}
/*****************************************
经典递归算法【求斐波那契数列】
******************************************/
int Fib(int n)
{
	int retval;
	if (n <= 0) {    //基本情况
		retval = 0;
	}
	else {
		if (n == 1) { // 基本情况
			retval = 1;
		}
		else {
			retval = Fib(n - 1) + Fib(n - 2);
		}
	}
	return retval;
}
int test2(void)
{
	return Fib(10);
}
/*****************************************
经典动态规划【01背包问题】
******************************************/
int Max(int a, int b) {
	if (a >= b) {
		return a;
	}
	else {
		return b;
	}
}
int getAns(int i, int wi) {
	int x;
	x = 0;
	while (x <= i)
	{
		dp[x][0] = 0;
		x = x + 1;
	}
	x = 0;
	while (x <= wi)
	{
		dp[0][x] = 0;
		x = x + 1;
	}
	x = 1;
	while (x <= i) {
		int y;
		y = 1;
		while (y <= wi) {
			if (y >= w[x - 1]) {
				dp[x][y] = Max(dp[x - 1][y], v[x - 1] + dp[x - 1][y - w[x - 1]]);
			}
			else {
				dp[x][y] = dp[x - 1][y];
			}
			y = y + 1;
		}
		x = x + 1;
	}
	return dp[i][wi];
}
int test3(void)
{
	v[0] = 6; v[1] = 3; v[2] = 5; v[3] = 4; v[4] = 6;
	w[0] = 2; w[1] = 2; w[2] = 6; w[3] = 5; w[4] = 4;
	return getAns(5, 10);
}
/*****************************************
单项组合测试【布尔表达式、赋值表达式测试、经典交换算法】
******************************************/
int test4(void)
{
	int t1;
	int t2;
	int t3;
	int retval;

	//测试简单布尔表达式与赋值语句 结果t1=1,t2=2,t3=3
	t1 = (t2 = 1);
	t3 = (t1 != t2);
	t2 = ((!t1) == 0) + ((t3 + 1) != 0);
	t3 = (t2 * (t3 + 1) >= t2) * t2 + ((t1 <= t3 + 1) == 1);
	t3 = t3 / 2;
	t3 = t3 * 2 + 1;

	//测试加减运算符 t1与t3交换
	t1 = t1 + t3;
	t3 = t1 - t3;
	t1 = t1 - t3;

	//测试异或运算符 t2与t3交换
	t2 = t2 ^ t3;
	t3 = t2 ^ t3;
	t2 = t2 ^ t3;

	//测试复杂布尔表达式
	retval = ((t1 == 3) && !(t2 != 1 || t3 != 2));
	return retval;
}
/*****************MAIN********************/
void main(void)
{
	int answer_count;
	int test_num;
	test_answer[0] = test0();
	test_answer[1] = test1();
	test_answer[2] = test2();
	test_answer[3] = test3();
	test_answer[4] = test4();
	answer_count = 0;
	test_num = 5;
	sum_answer = 0;
	while (answer_count < test_num)
	{
		sum_answer = sum_answer + test_answer[answer_count];
		answer_count = answer_count + 1;
	}
	return;
}