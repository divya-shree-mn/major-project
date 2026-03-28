
int main()
{
  int a, b, c;
  b = 10;
  c = 15;


  //   F
  if (c == 10)
  {
    a = 5;
  }

  //   F
  if (c == 10)
  {
    a = 10;
  }
  else
  {
    a = 15;
  }

  //    F         T
  if (c == 10 || b == 10)
  {
    a = 10;
  }

  //    T          T (not taken)
  if (c == 15 || b == 10)
  {
    a = 10;
  }

  //     T        T
  if (c == 15 && b == 10)
  {
    a = 10;
  }

  //     T         F
  if (c == 15 && b == 15)
  {
    a = 10;
  }

  // F           T
  if (c == 10 && b == 15)
  {
    a = 10;
  }
}
