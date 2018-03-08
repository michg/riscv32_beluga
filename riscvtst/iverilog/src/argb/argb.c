struct tag {
    int x[2];
    char c;
    char d;
    unsigned char e[3];
};

void f(struct tag x)
{
    puti(x.x[0]);
	puti(x.x[1]);
	puti((int)x.c);
}

int main(void)
{
    struct tag s = { 1, 2, 255, -128, -1, 255, -128 };
    f(s);
}
