translate([5, 5, 0])
    cube(10);

translate([-10, 10, 5])
    sphere(5);
    
translate([-10, -10, 5])
    rotate([0, 0, 45])
        cylinder(h=10, r1=5, r2=0, center=true, $fn=4);
           
translate([10, -10, 0])
    difference()
    {
        cylinder(h=10, r=5, center=false, $fn=15);
        translate([0, 0, -1])
            cylinder(h=12, r=3, center=false, $fn=15);
    }
