String name = "image.jpg" ; // inside "data" folder
int out_w = 60;
int out_h = 60;

int mult = 6;

PImage img;

void setup() {
  noSmooth();
  size(out_w*3*mult, out_h*mult);

  // The image file must be in the data folder of the current sketch 
  // to load successfully
  img = loadImage(name);  // Load the image into the program 
  image(img, 0, 0, height, height);
  img.resize(out_w, out_h);
  image(img, height, 0, height, height);

  out_start();

  img.loadPixels();
  for (int line = 0; line<out_h; line++) {
    out_line_start(line);
    for (int column = 0; column<out_w; column++) {
      color pixel = img.pixels[column+line*out_w];

      out_val(rrrggbbb(pixel), column);

      noStroke();
      //      stroke(0);
      fill(simulate(pixel));
      rect((column+out_w*2)*mult, line*mult, mult, mult);
    }
    out_line_end();
  }

  out_end();
}

void draw() {
}

color simulate (color pixel) {
  color result = 0;

  int rgb = rrrggbbb(pixel);

//  //rr0ggbb0
//  int r = ((rgb ) & 0xC0) >>6 ;
//  int g = ((rgb ) & 0x18) >>3;
//  int b = ((rgb ) & 0x06)>>1;
//
//  r =  (int)map (r, 0, 3, 0, 255);
//  g =  (int)map (g, 0, 3, 0, 255);
//  b =  (int)map (b, 0, 3, 0, 255);

  //rrrggbbb
  int r = ((rgb ) & 0xE0) >>5 ;
  int g = ((rgb ) & 0x18) >>3;
  int b = ((rgb ) & 0x07)>>0;

  r =  (int)map (r, 0, 7, 0, 255);
  g =  (int)map (g, 0, 3, 0, 255);
  b =  (int)map (b, 0, 7, 0, 255);

  result = (r<<16) | (g<<8) | (b);

  return result | 0xFF000000;
}

int rrrggbbb (color pixel) {
//// rr0ggbb0
//  int r = (pixel >>16)&0xFF;
//  int g = (pixel >>8)&0xFF;
//  int b = (pixel >>0)&0xFF;
//  
//  r = (int) map (r,0,255,0,3);
//  g = (int) map (g,0,255,0,3);
//  b = (int) map (b,0,255,0,3);
//
//  int result = r <<6 | g <<3| b<<1;

// rrrggbbb
  int r = (pixel >>16)&0xFF;
  int g = (pixel >>8)&0xFF;
  int b = (pixel >>0)&0xFF;
  
  r = (int) map (r,0,255,0,7);
  g = (int) map (g,0,255,0,3);
  b = (int) map (b,0,255,0,7);


  int result = r <<5 | g <<3| b<<0;

  return result ;
}

color simulate_bw (color pixel) {
  color result = 0;

  result = bw(pixel)<<8; //turns into green

  return result | 0xFF000000;
}


float rx,gx,bx;
  
int bw (color pixel) {
/*  rx = 0.21;
  gx = 0.71;
  bx = 0.071;
*/
/*
  rx = 0.5;
  gx = 0.419;
  bx = 0.081;
*/  
  rx = 0.299;
  gx = 0.587;
  bx = 0.114;

  int r = (pixel >>16)&0xFF;
  int g = (pixel >>8)&0xFF;
  int b = (pixel >>0)&0xFF;
  
   int result = int(float(r)*rx+float(g)*gx+float(b)*bx);

  return result ;
}

// file functions 
PrintWriter output;

void out_start() {
  output = createWriter("image.h");

  println ("char message ["+out_w+"] ["+out_h+"] PROGMEM = {");
  output.println ("char message ["+out_w+"] ["+out_h+"] /*PROGMEM*/ = {");
}
void out_end() {
  println ("};  //  end of image");
  output.println ("};  //  end of image");

  output.flush(); // Write the remaining data
  output.close(); // Finish the file
}
void out_line_start(int line) {
  println ("// line "+line);
  println ("  {");
  print ("    ");
  output.println ("// line "+line);
  output.println ("  {");
  output.print ("   ");
}
void out_line_end() {
  println ();
  println ("  },");
  output.println ();
  output.println ("  },");
}
void out_val(int value, int column) {
  if (column%8==0) {
    println();
    output.println();
  }
  print (" 0x"+hex(value, 2)+",");
  output.print (" 0x"+hex(value, 2)+",");
}
