package com.example.remote_controller;

import androidx.appcompat.app.AppCompatActivity;

import android.annotation.SuppressLint;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.bigkoo.pickerview.builder.OptionsPickerBuilder;
import com.bigkoo.pickerview.listener.OnOptionsSelectListener;
import com.bigkoo.pickerview.view.OptionsPickerView;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttConnectOptions;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.net.HttpURLConnection;
import java.text.DateFormat;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.Response;


public class MainActivity extends AppCompatActivity {

    private String set1="",set2="",set3="";
    private TextView outsideweather,outsidetemperature,Temperature1,Humidity1,Temperature2,Humidity2,CO2;
    private Button Deng,Jinbao,Fengsan;
    private String Weather,Tempeature;
    private int gettemperature[] = new int[3];
    private int gethumidity[] = new int[3];
    private String host= "tcp://118.178.90.222:1883";
    private ScheduledExecutorService scheduler;
    private String userName = "android1";
    private String passWord = "android1";
    private String mqtt_id = "MyZigbeeProject_android";
    private String mqtt_sub_topic1 = "/MyZigbeeProject/EndDevice1";
    private String mqtt_sub_topic2 = "/MyZigbeeProject/EndDevice2";
    private String mqtt_sub_topic3 = "/MyZigbeeProject/Router3";
    private String mqtt_sub_topic4 = "/MyZigbeeProject/IndustryData";
    private String mqtt_pub_topic1 = "/MyZigbeeProject/Control";
    private MqttClient client;
    private MqttConnectOptions options;
    private Handler handler;

    @SuppressLint("HandlerLeak")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        outsidetemperature = findViewById(R.id.outsidetemperature);
        outsideweather = findViewById(R.id.outsideweather);
        Temperature1 = findViewById(R.id.temperature1);
        Humidity1 = findViewById(R.id.humidity1);
        Temperature2 = findViewById(R.id.temperature2);
        Humidity2 = findViewById(R.id.humidity2);
        CO2 = findViewById(R.id.co2);
        Deng = findViewById(R.id.deng);
        Jinbao = findViewById(R.id.jinbao);
        Fengsan = findViewById(R.id.fengsan);


        for(int i = 0;i<3;i++){
            gettemperature[i] = 0;
            gethumidity[i] = 0;
        }

        Deng.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showPickerView();
                sendRequestWithOkHttp();
            }
        });

        Jinbao.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showPickerView2();
                sendRequestWithOkHttp();
            }
        });

        Fengsan.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showPickerView3();
                sendRequestWithOkHttp();
            }
        });

  //    ******************************************************************************//

        Mqtt_init();
        startReconnect();


        handler = new Handler() {
            @SuppressLint({"SetTextI18n", "HandlerLeak"})
            public void handleMessage(Message msg) {
                super.handleMessage(msg);
                switch (msg.what){
                    case 1: //开机校验更新回传

                        break;
                    case 2:  // 反馈回传

                        break;
                    case 3:  //MQTT 收到消息回传   UTF8Buffer msg=new UTF8Buffer(object.toString());

                        try {
                            flash(msg.obj.toString());
                        } catch (JSONException e) {
                            e.printStackTrace();
                        }

                        break;
                    case 30:  //连接失败
                        Toast.makeText(MainActivity.this,"连接失败" ,Toast.LENGTH_SHORT).show();
                        break;
                    case 31:   //连接成功
                        Toast.makeText(MainActivity.this,"连接成功" ,Toast.LENGTH_SHORT).show();
                        sendRequestWithOkHttp();
                        try {
                            client.subscribe(mqtt_sub_topic1,0);
                            client.subscribe(mqtt_sub_topic2,0);
                            client.subscribe(mqtt_sub_topic3,0);
                            client.subscribe(mqtt_sub_topic4,0);
                        } catch (MqttException e) {
                            e.printStackTrace();
                        }
                        break;
                    default:
                        break;
                }
            }
        };
    }

    private void flash(String message) throws JSONException {
        String thetopic,thevalue;
        thetopic =  message.substring(0, message.indexOf("---"));
        thevalue =  message.substring(message.indexOf("---")+3);
        JSONObject jsonObject = new JSONObject(thevalue);
        int averagetemp = 0,averagehumid = 0,tempnum = 0,huminum = 0;
        if(thetopic.startsWith(mqtt_sub_topic1)){
            gettemperature[0] = Integer.parseInt(jsonObject.getString("temperture"));
            gethumidity[0] = Integer.parseInt(jsonObject.getString("humidity"));
        }else if(thetopic.startsWith(mqtt_sub_topic2)){
            gettemperature[1] = Integer.parseInt(jsonObject.getString("temperture"));
            gethumidity[1] = Integer.parseInt(jsonObject.getString("humidity"));
        }else if(thetopic.startsWith(mqtt_sub_topic3)){
            gettemperature[2] = Integer.parseInt(jsonObject.getString("temperture"));
            gethumidity[2] = Integer.parseInt(jsonObject.getString("humidity"));
        }else if(thetopic.startsWith(mqtt_sub_topic4)){
            Temperature2.setText("精准温度："+jsonObject.getString("temperture")+"℃");
            Humidity2.setText("精准湿度："+jsonObject.getString("humidity")+"%");
            CO2.setText("二氧化碳含量："+jsonObject.getString("co2")+"ppm");

        }
        for(int i = 0;i<3;i++){
            if(gettemperature[i] != 0) {averagetemp += gettemperature[i];tempnum++;}
            if(gethumidity[i] != 0) {averagehumid += gethumidity[i];huminum++;}
        }
        if(tempnum != 0) Temperature1.setText("环境平均温度："+averagetemp/tempnum+"℃");
        if(huminum != 0) Humidity1.setText("环境平均湿度："+averagehumid/huminum+"%");

    }

    private void Mqtt_init()
    {
        try {
            //host为主机名，test为clientid即连接MQTT的客户端ID，一般以客户端唯一标识符表示，MemoryPersistence设置clientid的保存形式，默认为以内存保存
            client = new MqttClient(host, mqtt_id,
                    new MemoryPersistence());
            //MQTT的连接设置
            options = new MqttConnectOptions();
            //设置是否清空session,这里如果设置为false表示服务器会保留客户端的连接记录，这里设置为true表示每次连接到服务器都以新的身份连接
            options.setCleanSession(false);
            //设置连接的用户名
            options.setUserName(userName);
            //设置连接的密码
            options.setPassword(passWord.toCharArray());
            // 设置超时时间 单位为秒
            options.setConnectionTimeout(10);
            // 设置会话心跳时间 单位为秒 服务器会每隔1.5*20秒的时间向客户端发送个消息判断客户端是否在线，但这个方法并没有重连的机制
            options.setKeepAliveInterval(20);
            //设置回调
            client.setCallback(new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {
                    //连接丢失后，一般在这里面进行重连
                    System.out.println("connectionLost----------");
                    //startReconnect();
                }
                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {
                    //publish后会执行到这里
                    System.out.println("deliveryComplete---------"
                            + token.isComplete());
                }
                @Override
                public void messageArrived(String topicName, MqttMessage message)
                        throws Exception {
                    //subscribe后得到的消息会执行到这里面
                    System.out.println("messageArrived----------");
                    Message msg = new Message();
                    msg.what = 3;   //收到消息标志位
                    msg.obj = topicName + "---" + message.toString();
                    //msg.obj = message.toString();
                    handler.sendMessage(msg);    // hander 回传
                }
            });
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void Mqtt_connect() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    if(!(client.isConnected()) )  //如果还未连接
                    {
                        client.connect(options);
                        Message msg = new Message();
                        msg.what = 31;
                        handler.sendMessage(msg);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                    Message msg = new Message();
                    msg.what = 30;
                    handler.sendMessage(msg);
                }
            }
        }).start();
    }

    private void startReconnect() {
        scheduler = Executors.newSingleThreadScheduledExecutor();
        scheduler.scheduleAtFixedRate(new Runnable() {
            @Override
            public void run() {
                if (!client.isConnected()) {
                    Mqtt_connect();
                }
            }
        }, 0 * 1000, 10 * 1000, TimeUnit.MILLISECONDS);
    }

    private void publishmessageplus(String topic,String message2)
    {
        if (client == null || !client.isConnected()) {
            return;
        }
        MqttMessage message = new MqttMessage();
        message.setPayload(message2.getBytes());
        try {
            client.publish(topic,message);
        } catch (MqttException e) {

            e.printStackTrace();
        }
    }


//    ******************************************************************************//
    /**
     * 展示选择器
     * 核心代码
     */
    private void showPickerView() {
//      要展示的数据
        final List<String> listData = getData();
//      监听选中
        OptionsPickerView pvOptions = new OptionsPickerBuilder(MainActivity.this, new OnOptionsSelectListener() {
            @Override
            public void onOptionsSelect(int options1, int option2, int options3, View v) {
//               返回的分别是三个级别的选中位置
//              展示选中数据
                if(listData.get(options1) == "自动" ){
                    set1 = "0099";
                }else if(listData.get(options1) == "开" ){
                    set1 = "0199";
                }else if(listData.get(options1) == "关" ){
                    set1 = "0299";
                }

                publishmessageplus(mqtt_pub_topic1,set1);
                System.out.println(set1);
                Deng.setText("设置大灯工作模式："+listData.get(options1));

            }
        })
                .setSelectOptions(0)//设置选择第一个
                .setOutSideCancelable(false)//点击背的地方不消失
                .setContentTextSize(35)
                .build();//创建
//      把数据绑定到控件上面
        pvOptions.setPicker(listData);
//      展示
        pvOptions.show();

    }

    private void showPickerView2() {
//      要展示的数据
        final List<String> listData = getData();
//      监听选中
        OptionsPickerView pvOptions = new OptionsPickerBuilder(MainActivity.this, new OnOptionsSelectListener() {
            @Override
            public void onOptionsSelect(int options1, int option2, int options3, View v) {
//               返回的分别是三个级别的选中位置
//              展示选中数据
                //cloud.setText("设置风速为："+listData.get(options1));
                if(listData.get(options1) == "自动" ){
                    set2 = "0909";
                }else if(listData.get(options1) == "开" ){
                    set2 = "0919";
                }else if(listData.get(options1) == "关" ){
                    set2 = "0929";
                }
                publishmessageplus(mqtt_pub_topic1,set2);
                System.out.println(set2);
                Jinbao.setText("设置警报工作模式："+listData.get(options1));

            }
        })
                .setSelectOptions(0)//设置选择第一个
                .setOutSideCancelable(false)//点击背的地方不消失
                .setContentTextSize(35)
                .build();//创建
//      把数据绑定到控件上面
        pvOptions.setPicker(listData);
//      展示
        pvOptions.show();
    }

    private void showPickerView3() {
//      要展示的数据
        final List<String> listData = getData();
//      监听选中
        OptionsPickerView pvOptions = new OptionsPickerBuilder(MainActivity.this, new OnOptionsSelectListener() {
            @Override
            public void onOptionsSelect(int options1, int option2, int options3, View v) {
//               返回的分别是三个级别的选中位置
//              展示选中数据
                //cloud.setText("设置风速为："+listData.get(options1));
                if(listData.get(options1) == "自动" ){
                    set3 = "0990";
                }else if(listData.get(options1) == "开" ){
                    set3 = "0991";
                }else if(listData.get(options1) == "关" ){
                    set3 = "0992";
                }
                publishmessageplus(mqtt_pub_topic1,set3);
                System.out.println(set3);
                Fengsan.setText("设置风扇工作模式："+listData.get(options1));

            }
        })
                .setSelectOptions(0)//设置选择第一个
                .setOutSideCancelable(false)//点击背的地方不消失
                .setContentTextSize(35)
                .build();//创建
//      把数据绑定到控件上面
        pvOptions.setPicker(listData);
//      展示
        pvOptions.show();
    }

    /**
     * 造一组数据
     */
    private List<String> getData() {
        List<String> list = new ArrayList<>();
        list.add("自动");
        list.add("开");
        list.add("关");
        return list;

    }

    //*********************************//
    private void sendRequestWithOkHttp() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    OkHttpClient client = new OkHttpClient();//创建一个OkHttp实例
                    Request request = new Request.Builder().url("https://api.seniverse.com/v3/weather/now.json?key=SBs65mWIS80kXGG9L&location=guangzhou&language=zh-Hans&unit=c").build();//创建Request对象发起请求,记得替换成你自己的key
                    Response response = client.newCall(request).execute();//创建call对象并调用execute获取返回的数据
                    String responseData = response.body().string();
                    showResPonse(responseData);//显示原始数据和解析后的数据
                    parseJSONWithJSONObject(responseData);//解析SSON数据
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }).start();
    }

    private void parseJSONWithJSONObject(String jsonData) {//用JSONObect解析JSON数据
        try {
            JSONObject jsonObject = new JSONObject(jsonData);
            JSONArray results = jsonObject.getJSONArray("results");   //得到键为results的JSONArray
            JSONObject now = results.getJSONObject(0).getJSONObject("now");//得到键值为"now"的JSONObject
            Weather = now.getString("text");//得到"now"键值的JSONObject下的"text"属性,即天气信息
            Tempeature = now.getString("temperature"); //获取温度
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    private void showResPonse(final String response) {
        runOnUiThread(new Runnable() {//切换到主线程,ui界面的更改不能出现在子线程,否则app会崩溃
            @Override
            public void run() {
                outsideweather.setText("当前广州室外天气："+Weather);
                outsidetemperature.setText("当前广州室外温度："+Tempeature+"℃");
            }
        });
    }

}
