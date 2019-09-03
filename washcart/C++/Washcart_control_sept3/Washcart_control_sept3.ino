//alright so this code is intended to run on a node mcu and control one of techmists washcart
//intended features incude PID motor control, brake control, valve/flow control, 
//two way connection with a techmist gateway, possibly wlan but most likely 
//make changes to current gateway wlan in order to be able to configure in same 
//way as a treatment but its a new tab for washcart 

//code is intended to be structured roughly as follows
//first is the setup, in here we wait for data from the wlan, once adequet information 
//has been recieved we can initiate the "washing" or "wash treatment"

//starting the wash begins with determining the current state of the things we control
//this meens polling/checking sensors. the first time this is done many of the will reutrn 
//0's or default variables as we havnt begun doing anything yet.
//as time goes on and we revist this part values will get changed/updated

//once we have the current data we need to determine the desired data, and associated variables to get there. 
//for many sensors and parameters the desired values are determined by inputs to the WLAN
//others however must be calculated.values like the k's for a PID and the acc to get
//desired speeds, valve opening procedures stuff like that to fill in the gaps left 
//by user input andensure smooth transitions between states. 
//some values are complete hardcoded logic to simplify the user interface and selections
//example. user selects number off passes(and types) aswell as maybe speed. they dont
//need to specify acceleration for the direction changes

//With all the desired values for the control of the system the next thing that occours
//is to apply the values. each system will have its own function for changing its preformance
//each of these function will take current and desired variables aswells as
//possibly a few other and do what must be done to get the attribute they control
//on path to making current and desired values the same

//there will also be a few external interupts for sensors that we need to deal with
//immediatly
//one such sensor is the range limiter which when toggled needs to get the cart to turn around


//wlan should have num of passes state for each of these passes and.....




int region_state = 0;//movement regions are where on rail we are start middle end (0 = start, 1 = middle 2 = end)//might be able to get away with comning 0 and 1 with pid
bool movement_dir = false;//false = forward true = backward
String user_wash_vars[10] = {"-1","-1","-1","-1","-1","-1","-1","-1","-1","-1"};//stores the variables that are specified in the WLAN
//^doesnt change a whole lot so maybe i could make it static in some form?
//the data is stored in this array in form {identifier,dataname,dataname,dataname....}
//could maybe remove the identifier
int state_counter = 0;//counts the number of time we have gone a full length of the rail
//^could possibly be done by doing a bunch of variable passing instead of global
int hall_effect_triggers = 0; //this variable increase everythime the hall effect sensor interupt triggers
int hall_effect_timer = 0; //used to track the time between hall effect triggers
int hall_effect_speed = 0; //is the amount of time between two triggers of hall effect snesor
int current_region_len;//holds the value for the len of the movement region we are in
void setup() {
  // pretty much just pin setups and serial begins
    Serial.begin(115200);//used for xbee(uart 0,1,2 could all be used i picked 0 so i see all prints on monitor)
  
}//end of setup




void loop() {// yall already know what a main loop is
//variable declares for the main while(1) loop "global"
  int wash_state =0;//different numbers for dif states (list states here)
  int last_wash_state = 0;//used so we dont always go through wash state change if we are changing to the state we are already in
  bool wash_data_recieved = false;//used to make code wait until we have recieved infor from wlan
  String temp_in_array[10] = {"-1","-1","-1","-1","-1","-1","-1","-1","-1","-1"};//temporarly stores whatever comes in over UART until we decided where to put data
  String reset_array[10] = {"-1","-1","-1","-1","-1","-1","-1","-1","-1","-1"};//used to reset temp_in_array
  int actuator1_start =0;//used for timing how long actuator is on for
  int actuator1_dir= 0; //0=forward 1 = backward
  int actuator1_state = 0;//0 = up(not moving),1 = up(moving),2=down(staying still),3(down moving)
  int motor_state = 0;// 0 = off 1 = forward 2 = backward
  int desired_motor_speed = 0;//used as input to pid//compared with speed derived from hall effect trigger
  int current_distance = 0;//based on hall effect data
  int current_speed = 0;//based on hall effect data
  int current_acc = 0;//based on hall effect snesor data
  int state_distance = 0;//this guy determines how far a single pass is end to end

//wait for info from wlan
   while(wash_data_recieved=false)
   {
    int i = 0; //used to itterate through the array we store the user defined wash variables in
      while(Serial.available())//recieves from the xbee, the setup data
      {
          char received = Serial.read();//used to grab every individual char sent over uart
          if(received ==',')//requires comma at end of string can fix be putting one itteration of loop in begining og /n handler
          {
             temp_in_array[i] = temp_data;
             temp_data = "";
             i++;
          }//end of , loop
          else{temp_data+=received;}

          if(received =='\n')//recieved end of incoming datastring, now time to find out what it is
          {
          identifier_match(temp_in_array);
          i = 0; //need to reset counter
          equal_array(temp_in_array,reset_array)//need to reset the temp array
          }//end of /n loop
      }//end of serial avaialable loop
   }//end of wait for information about wash loop



  while(1)//helps with scoping and allows 1 time things on start of main loop
  {//essentially the "real main loop"
//first get data 
    poll_pressure_sensors();//grabs values of the pressure sensors
//most other data is either input once in begining or comes in from interupts
    calculate_dist_speed_and_acc(&current_distnace,&current_speed,&current_acc);//based on hall effect data calculate distance speed and acc

//next get desired values based on data and user input
//this is where we would plug in a PID
    if(current_distance>=state_distance)//if we have gone enough distance to be at end of region
    {//this guy is main control of when to swap states
      wash_state = desired_wash_state();//should only call when we reach certian distnace treshholds
      if(last_wash_state != wash_state)//detects change in wash state
      {
        handle_wash_states(wash_state);//this guy sets the desired states and tracks the states of other parts(like whether actuator should be on or not)
      }
    }//end of check if at end of state 


    if(actuator1_state != 0)
    { 
      actuator1_set(&actuator1_state,&actuator1_start,&actuator1_dir)//determine if actuator is on or off
    }


    if(motor_state != 0)//if not in off state
    {//basicly motor state is set in the handle wash states. once set it does a few thing
      motor_set(&motor_state, &motor_speed)//used to make acc and deacc smooth
    }
    
//actuator1_state is set in handle_wash_states 
//actuator1_start is also set in handle wash states



//next apply data (this includes changing speed)

actuator1_control()

last_wash_state = wash_state;




  }//end of while 1
}//end of main





void identifier_match(String *temp_in_array)//this function allows us to determine what happens to a certian data string recieved on the UART 
{
   delay(100);//this delay allows for computer to catch up a bit before it begins comparisons (had issues without it)

   if (String(temp_in_array[0]) == String("Wash_Data"))
   {//if temp_in_array is the information for starting a wash than we need to send it to the array that stores the wash setup data 
    equal_array(user_wash_vars,temp_in_array);//now user_wash variables has the data from user input
   return;//allows identifier funtion to terminate once it finds a match
   }//end of wash data handler
}//end of identifier_match






void equal_array(String *destination_array, String *data_array)//need to pass as just name of array no []
{//sets destination array eual to data array
  for(int i = 0; i < 10; i++)//I want to find a way to make this not care how big or small arrays are (so long as destination is larger than data)
  {
  destination_array[i] = data_array[i];//apperently you do not need to specefy these a pointers
  }
}//end of reset array



////////////////////functions for grabbing sensor data//////////



//////////polling functions////////
//flow sensor




/////////interupt handlers////////
//hall effect sensor,range limiter sensor
void hall_effect_handler()//checks if we have gone far enough to trigger a region change
{
  hall_effect_triggers = hall_effect_triggers++;
  hall_effect_speed = millis()-hall_effect_timer;
  hall_effect_timer = millis();
}//end of hall handler




void distnace_limit_handler()//range limit sensor handler that sets the current regions state
{//might be scrapped but needs to line up with hall effect trigger
region_state_toggle();//toggle the region we are in
}//end of distance limit handler




void region_state_toggle()//this function is here because sometimes the state is changed based on calls from things that are not the limit
{

  
if(region_state >=2)
{
  movement_dir = !movement_dir;//whenever we get to the end of a pass we must swap directions(end of pass call to this function should come from whatever is controling the end region(csall when it thinks its done))
  region_state = 0;
  num_passes = num_pass++;
}else{region_state++;}


}//end of region_state_toggle

void calculate_dist_speed_and_acc()
{
  
  }






//////////////////functions for desired data////////////////
//if we want flow rate to change over time or we want to do any type of acc its calculated here
//has the functions that change behevior of system dependednt of region (start middle end)
//end region(start and end) are used as acc and dec regions so we arnt doing instant start stops
//also has counter for number of passes so we can switch between type of spray(types of spray will be states here)

void desired_wash_state()//this function looks at the user input variables and based on current 
{//determines wash state based on position and num of states completed

return(user_wash_vars[x+state_counter].toInt);//x is the index where we say how many passes we do
//order of wash states is set by wlan based on user selection



}



actuator1_set(int *actuator1_state, int *actuator1_start, int *actuator1_dir)
{//this function dictates what the actuator should do once it has been set into state 1
//basicly we stay in each state for a certian period of time and than swap to the next state

  
  int movement_time = //amount of time we want the actuator to be on for
  if(actuator_state = 1)
  {
    if(millis()-actuator1_start>=movement_time)
    {//if actuator state 1 and we ran for this amount of time swap to state 2
        actuator_state = 2;
        actuator_start=millis();
    }
  }//end of state 1 loop

  if(actuator_state = 2)
  {//in state 2 we wait at bottom spraying for a certian period of time
    if(millis()-actuator1_start>=movement_time/10)
    {
        actuator_state = 3;
        actuator_start=millis(); 
        actuator_dir = 1; 
    }
  }//end of state 2 loop

  if(actuator_state = 3)
  {//in state 2 we wait at bottom spraying for a certian period of time
    if(millis()-actuator1_start>=movement_time)
    {
        actuator_state = 0;
        actuator_dir = 0;//want to reset this into the forward position  
    }
  }//end of state 2 loop

}//end of actuator 1 set

void motor_set(int *motor_state, int *motor_speed,state_distance)
{//this guy takes into account its current distance into a pass and does things dep on that
  //main things are acc and deacc neer ends
 if(millis()-actuator1_start>=movement_time)
    {//if actuator state 1 and we ran for this amount of time swap to state 2
        actuator_state = 2;
        actuator_start=millis();
    }
  }//end of state 1 loop

  
}



//handler for dif wash states 
void handle_wash_states(int wash_state)//gets called by main
{//navigation between wash states is handled by desired_washstate

   //these states are triggered based on location, they are triggered only once apon hitting said location
   //the intended purpose is that these cases will set the states and conditions for the 
   //other componenets to behave appropriatly for this specific location
   //actuators and motor performance are started here
   
switch (wash_state) {
  //7 states 6 valves, 6 passes
   case 0://each case specifies settings for the nosles and pressure and hose for dif tratemnet aspects
      //this state is the off state
      //do we turn off as we move actuator
    break;
    case 1:
       //this is the acc state, it is used to get us up to speed
      break;
  case 1://each case specifies settings for the nosles and pressure and hose for dif tratemnet aspects
    // for this state we foam the cieling (middle of pass 1)
    //should set movement action here awsell
    break;
    case 2:
    //foam wall(end region pass 1)
    break;  
  case 3:
    //foam gutters on way back with valve 2
    break;
      case 4:
    //rinse roof and gutters with wall using valve 3 which is part of valve 1
    break;
      case 5:
    //wash gutters using valve 4
    break;
      case 6:
    //wash underside of gutters valve 5
    break;
      case 7:
    //floor sweep with aqua broom valve 6
    break;


}//end of case state
}//end of handler







///////////////functions for feedback application///////////////////
//will have the actuall PID controller in it
//valve control functions
//state sorter. we determine the state in desired data section here is where what we do in that state is specified


void set_nossle()//this guy does what needs to be done to nossle valve(called by main)
{
  
}//end of set nossle


actuator1_control(int start_time,int *actuator1_dir)//ths function controls when to turn actuator on and off as well as what direction to move it in  
{
//I could put an if sorter here for different states of the actuator control/does different things(would need a variable input)



  
}
