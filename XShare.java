import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;

public class XShare
{
    //数组下发 赋值 
    private native int J2C_WrBuffer(byte[] arr);
    private native int C2J_WrCache(byte[] arr);
    //数组上传 取值
    private native byte[] J2C_RdBuffer();
    private native byte[] C2J_RdCache();

    static{
        System.out.println(System.getProperty("java.library.path"));
     	System.load("/root/Esl5/ProXShare/Debug/libProXShare.so");    //使用绝对路径加载名称为goodluck.so的库文件
    }

    private static int x = 13000;

    public static void main(String[] args)
    {
        XShare h = new XShare();
	Timer timer = new Timer();  
        timer.schedule(new TimerTask() {  
            public void run() {  
		
		//write                
		
		byte[] arr = new byte[x];
		for (int i = 0; i < arr.length; i++) {
			//arr[i] = (byte)((i+6) & 0xFF);
			if(i<3500)	arr[i] = 2;
			else if((i>=3500) && (i<7000)) arr[i] = 100;
			else		arr[i] = 78;	
		}
		int sum = h.J2C_WrBuffer(arr);
		//System.out.println("sum = " + sum + "  x = " + x);
		//x = x + 1;
		//h.sayHello();
		//System.out.println("现在是：" + new Date());
		

		//READ
		/*
		byte[] result = h.J2C_RdBuffer();
		String str = "Get res:";	
		for(int i = 0, s = result.length; i < s; i++) {
		    str += ((int) result[i]);
	            str += " ";
		}
		System.out.println(str);
		*/
            }  
        }, 10,10);// 设定指定的时间time,此处为2000毫秒  
    }

}



