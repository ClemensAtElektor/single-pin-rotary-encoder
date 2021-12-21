/*
 * Project: 190097 Single-pin Rotary Encoder
 * Description:
 * Creation date: December 19, 2019
 * Author: Elektor, CPV
 * (C) Elektor, www.elektor.com
 * License: GPL-3.0
 */

const char *p_project = "190097 Single-pin Rotary Encoder";
const uint8_t version_hi = 0;
const uint8_t version_lo = 1;

// AD converter look-up table to handle input offset and imprecise
// R-2R networks. Adapt for your circuit.
const int bounds[64] =
{
   20,  36,  51,  67,  83,  98, 114, 130,
  144, 160, 176, 192, 207, 223, 239, 255, 
  271, 286, 302, 318, 334, 350, 366, 381, 
  396, 412, 428, 444, 459, 475, 491, 507, 
  520, 535, 551, 567, 582, 598, 614, 630, 
  644, 660, 676, 691, 707, 723, 739, 755, 
  771, 787, 803, 819, 834, 850, 866, 882, 
  896, 912, 928, 944, 960, 976, 992, 1007
};

typedef struct
{
  int state;
  int state_sub;
}
rotary_encoder_t;

rotary_encoder_t s1;
rotary_encoder_t s2;

void splash(void)
{
  Serial.print("Elektor project: ");
  Serial.println(p_project);
  Serial.print("Version ");
  Serial.print(version_hi);
  Serial.print('.');
  Serial.println(version_lo);
  Serial.println("https://www.elektor.com\n");
}

int encoder_read(rotary_encoder_t &s, uint8_t pin_states)
{
  int result = 0;
  
  // Make sure state is 0, 1, 2 or 3.
  pin_states &= 0x03;
  
  if (pin_states!=s.state)
  {
    // Exor the old & new states to determine the rotation direction.
    int inc = ((pin_states>>1)^s.state)&0x01;
    if (inc==0) inc = -1;
    s.state = pin_states;

    // Reset on change of direction.
    if ((inc<0 && s.state_sub>0) || (inc>0 && s.state_sub<0)) s.state_sub = 0;

    s.state_sub += inc;
    if (s.state_sub<=-4 || s.state_sub>=4)
    {
      s.state_sub -= (inc<<2);
      result = inc;
    }
  }
  
  return result;
}

void setup(void)
{
  Serial.begin(115200);
  splash();
  // Init rotary encoder data structures.
  memset(&s1,0,sizeof(s1));
  memset(&s2,0,sizeof(s2));
}

void loop(void)
{
  static int counter1_last = 0;
  static int counter1 = 0;
  static int counter2_last = 0;
  static int counter2 = 0;
  
  int v = analogRead(A0);

  // Map input voltage to range.
  int j = 0;
  while (v>bounds[j])
  {
    j += 1;
  }
  if (j>0)
  {
    int a = bounds[j] - v;
    int b = v - bounds[j-1];
    if (a>b) j -= 1;
  }

  if (j>0)
  {
    // Clear counter on pushbutton press.
    if ((j&0x04)==0) counter1 = 0; // pushbutton of S1 on bit 2.
    if ((j&0x20)==0) counter2 = 0; // pushbutton of S2 on bit 5.
    counter1 += encoder_read(s1,j); // signals 'A' & 'B' of S1 on bits 0 & 1.
    counter2 += encoder_read(s2,j>>3); // signals 'A' & 'B' of S2 on bits 3 & 4.
  }

  // Update display only when a counter was modified.
  if (counter1!=counter1_last || (counter2!=counter2_last))
  {
    Serial.print(counter1);
    Serial.print("\t");
    Serial.print(counter2);
    Serial.println();
  }
  counter1_last = counter1;
  counter2_last = counter2;
}

