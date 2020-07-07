void HW_TDMA_Start(void)
{
   HW_WRITE(PDN_CLR2, PDN_CON_TDMA); /*power on TDMA*/   
}

void WAIT_TIME_QB( short time_qb )
{
   unsigned short  last_qb, cur_qb;
   short  qb_diff, qb_remain;

   qb_remain = time_qb;
   last_qb = HW_READ( TDMA_TQCNT );
   while( qb_remain>0 )
   {  cur_qb = HW_READ( TDMA_TQCNT );
      qb_diff = cur_qb-last_qb;
      qb_remain -= (qb_diff<0) ? 1 : qb_diff;
      last_qb = cur_qb;
   }
}
