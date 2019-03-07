
int CHANNEL_PER_PMT = 64;
int PMT_ROWS = 18;
int PMT_COLUMNS = 6;

int GetPmtID( int channel ) 
{
	int MAX_BOX_CHANNEL = CHANNEL_PER_PMT * PMT_ROWS * PMT_COLUMNS;
	if(channel < MAX_BOX_CHANNEL) 
		return channel/CHANNEL_PER_PMT;
	else 
		return (channel-MAX_BOX_CHANNEL)/CHANNEL_PER_PMT;
}

int GetPmtColumn( int channel )
{
	int pmt = GetPmtID(channel);
	return pmt/PMT_ROWS; //  0 - 5
}

int GetPmtRow( int channel ) 
{
	int pmt = GetPmtID(channel);
	return pmt%PMT_ROWS; //  0 - 17
}

void writeDummy() {

  for(int i=0; i<6912; i++) {
	int row = GetPmtRow(i);
	int col = GetPmtColumn(i);
	if((col == 0 && row < 11) || (col == 5 && row < 7))
		cout<<1<<endl;
	else
		cout<<0<<endl;
  } 

  return;
}
