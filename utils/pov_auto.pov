//-*- indent-tabs-mode: nil; tabs: 3 -*-
// Draw a 3d automata using povray
// pov_automata.pov, Copyright (c) 2003 Jeremy English <jenglish@myself.com>
//
// Permission to use, copy, modify, distribute, and sell this software and its
// documentation for any purpose is hereby granted without fee, provided that
// the above copyright notice appear in all copies and that both that
// copyright notice and this permission notice appear in supporting
// documentation.  No representations are made about the suitability of this
// software for any purpose.  It is provided "as is" without express or 
//  implied warranty.
//
// Examples taken from Stephen Wolfram's book "A New Kind Of Science"
// pages 182, 183

#include "colors.inc"
#include "golds.inc"

#declare twentysixcell = array [26] [3] {
  {0, 0, 1},
  {0, 0, -1},
  {0, 1, 0},
  {0, 1, 1},
  {0, 1, -1},
  {0, -1, 0},
  {0, -1, 1},
  {0, -1, -1},
  {1, 0, 0},
  {1, 0, 1},
  {1, 0, -1},
  {1, 1, 0},
  {1, 1, 1},
  {1, 1, -1},
  {1, -1, 0},
  {1, -1, 1},
  {1, -1, -1},
  {-1, 0, 0},
  {-1, 0, 1},
  {-1, 0, -1},
  {-1, 1, 0},
  {-1, 1, 1},
  {-1, 1, -1},
  {-1, -1, 0},
  {-1, -1, 1},
  {-1, -1, -1}
};

#declare BUF_SZ=11;
#declare HALF_SZ=5;
#declare cell_sum = 0;

#declare buffer = array[BUF_SZ] [BUF_SZ] [BUF_SZ];
#declare prebuffer = array[BUF_SZ] [BUF_SZ] [BUF_SZ];

#declare n_pbuf = 1;
#declare n_buf  = 0;

#macro init_buffers()
  #local i = 0;
  #local j = 0;
  #local k = 0;
  #while (i < BUF_SZ)
    #local j = 0;
    #while (j < BUF_SZ)
      #local k = 0;
      #while (k < BUF_SZ)
        #declare prebuffer [i] [j] [k] = 0;
        #declare buffer [i] [j] [k] = 0;
        #local k = k + 1;
      #end
      #local j = j + 1;
    #end
    #local i = i + 1;
  #end
#end

 #macro unit_cube()
  superellipsoid { <0.375, 0.375> 
    scale <0.45,0.45,0.45>
  }
  //box{
  //  <0.40,0.40,0.40>, <-0.40,-0.40,-0.40>
  //}
#end


#macro draw_box(_x,_y,_z)
  object{
    unit_cube()
    translate <_x, _y, _z>

    texture{ 
	pigment { rgb <1,1,1> } 
    }
    finish { specular 0.9 roughness 0.02 ambient 0.5}
  }
#end

//Sorry about this. Does povray have and, or, and not statments?
#macro check_cells(_x, _y, _z)
  #declare cell_sum = 0;
  #local  i = 0;
  #while (i < 26)
    #local x1 = _x + HALF_SZ + twentysixcell[i][0];
    #local y1 = _y + HALF_SZ + twentysixcell[i][1];
    #local z1 = _z + HALF_SZ + twentysixcell[i][2];
    #if (x1 > -1 )
      #if (x1 < BUF_SZ)
        #if (y1 > -1)
          #if (y1 < BUF_SZ) 
            #if (z1 > -1)
              #if (z1 < BUF_SZ)
                #if (prebuffer[x1] [y1] [z1] = 1)
                  #declare cell_sum = cell_sum + 1;
                #end
              #end
            #end
          #end
        #end
      #end
    #end
    #local i = i + 1;
  #end
#end

#macro build_display ()
  #fopen log_file "pov_automata.log" write
  #local i = -HALF_SZ;
  #while (i <= HALF_SZ)
    #local j = -HALF_SZ;
    #while (j <= HALF_SZ)
      #local k = -HALF_SZ;      
      #while (k <= HALF_SZ)
        #local x1 = i + HALF_SZ;
        #local y1 = j + HALF_SZ;
        #local z1 = k + HALF_SZ;
        #write(log_file,x1,y1,z1,buffer[x1][y1][z1], "\n")
        #if (buffer[x1] [y1] [z1] = 1 )
          draw_box (i, j, k)
        #end
        #local k = k + 1;
      #end
      #local j = j + 1;
    #end
    #local i = i + 1;
  #end
#end

#macro turn_cell_on (bn, _x, _y, _z)
  #local x1 = _x + HALF_SZ;
  #local y1 = _y + HALF_SZ;
  #local z1 = _z + HALF_SZ;
  #if (bn = n_pbuf)
    #declare prebuffer[x1][y1][z1] = 1;
    #debug "\n"
    #debug "Turning on a cell in prebuffer\n"
  #else
    #declare buffer[x1][y1][z1] = 1;
    #debug "\n"
    #debug "Turning on a cell in buffer\n"
  #end
#end

#macro check_rule_1 (_x,_y,_z)
  #if (cell_sum = 1)
      turn_cell_on (n_buf,_x, _y, _z)
  #end
#end

#macro rule1_init()
  turn_cell_on(n_pbuf,0,0,0)
  turn_cell_on(n_buf,0,0,0)
#end

#macro cell_walk ()
  #local i = -HALF_SZ;
  #while (i <= HALF_SZ)
    #local j = -HALF_SZ;
    #while (j <= HALF_SZ)
      #local k = -HALF_SZ;      
      #while (k <= HALF_SZ)
        check_cells(i,j,k)
        check_rule_1(i,j,k)
        #local k = k + 1;
      #end
      #local j = j + 1;
    #end
    #local i = i + 1;
  #end
#end

#macro copy_buffer ()
  #local i = 0;
  #while (i < BUF_SZ)
    #local j = 0;
    #while (j < BUF_SZ)
      #local k = 0;      
      #while (k < BUF_SZ)
        #declare prebuffer[i][j][k] = buffer[i][j][k];
        #local k = k + 1;
      #end
      #local j = j + 1;
    #end
    #local i = i + 1;
  #end
#end

#macro setup_scene()
light_source{
  <-50,50,-50> White

}

//Fade from the center
light_source{
  <0,0,0> White
  shadowless
}

camera{
  location <2.5,-2.25,-20>
  look_at <2.5,-2.25,0>
}

//wall
 plane { <0, 0, 1>, 10
    pigment {
      rgb <0.19,0.26,0.42>
    }

}

#end


#macro main()
  init_buffers()
  setup_scene()
  rule1_init()
  #local i = 0;
  #while (i < 8)
    cell_walk()
    copy_buffer()
    #local i = i + 1;
    #debug "\n"
    #debug "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!MAIN LOOP!!!!!!!!!!!!!!!!!!!\n"
  #end
  #debug "\n"
  #debug "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!Building display!!!!!!!!!!!!!!!!!!!\n"  
  union {
    build_display()
    rotate <-25,25,0>
  }
#end

main()
