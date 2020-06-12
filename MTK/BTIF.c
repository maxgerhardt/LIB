signed int BTIF_TurnOnPower(int a1, int a2)
{
  signed int result; // r0
  result = 2;
  if ( a2 )
  {
    if ( a2 == 1 )
      MEMORY[0xA0010328] = 2; //CONFIG_base
  }
  else
  {
    MEMORY[0xA0010318] = 2; //CONFIG_base
  }
  return result;
}

btif_dummy_wait = MEMORY[0xA0020000]; // GPIO_Base