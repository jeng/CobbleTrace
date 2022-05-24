#include "colors.inc"
#include "golds.inc"

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
    texture {
    	pigment {
      		rgb <0.19,0.26,0.42>
    	}
   }
}


#end

#macro unit_cube()
  box{
    <5,5,5>, <-5,-5,-5>
  }
#end

#macro my_cube(_x,_y,_z)
object{ 
    unit_cube()
    translate <_x, _y, _z>
    texture{ 
	pigment { rgb <1,1,1> } 
    }
    finish { specular 0.9 roughness 0.02 ambient 0.5}
 }
#end

#macro main()
  setup_scene()
#union{
  my_cube(0,0,0)
  rotate<-35,25,0>
}
#end

main()
