struct tag {
    int x[2];
    char c;
    char d;
    unsigned char e[3];
} x = { 1, 2, 255, -128, -1, 255, -128 };


    

int main(void)
{
	struct tag s;
	s = x;
    puti(s.x[0]);
	puti(s.x[1]);	
}
