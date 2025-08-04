use crate::temp_alarm::temp_async_server::TempAsyncClient;
use crate::temp_alarm::temp_server::TempClient;
use crate::temp_alarm::SensorReadResult;
use erpc_rust::client::{ClientManager, CodecConfig};

pub async fn run_client() -> Result<(), Box<dyn std::error::Error>> {
    println!("🚀 Starting eRPC Temperature Client...");

    let mut shared_client_manager = ClientManager::builder()
        .tcp_connection("127.0.0.1:40000")
        .codec(CodecConfig::Basic)
        .connect()
        .await?;
    println!("✅ Connection validated and ClientManager created");

    let mut temp_client = TempClient::new(&mut shared_client_manager);

    // Add a sensor
    let sensor_added = temp_client
        .add_sensor(1)
        .await
        .map_err(|e| format!("add_sensor error: {}", e))?;
    println!("✅ Sensor added via TempClient: {}", sensor_added);

    // Set interval for the sensor
    let interval_set = temp_client
        .set_interval(1, 2.5)
        .await
        .map_err(|e| format!("set_interval error: {}", e))?;
    println!("✅ Interval set via TempClient: {}", interval_set);

    // Set an alarm
    let alarm_set = temp_client
        .set_alarm(1, crate::temp_alarm::AlarmType::kHighAlarm, 30.0)
        .await
        .map_err(|e| format!("set_alarm error: {}", e))?;
    println!("✅ Alarm set via TempClient: {}", alarm_set);

    // Enable the alarm
    let alarm_enabled = temp_client
        .enable_alarm(1, crate::temp_alarm::AlarmType::kHighAlarm)
        .await
        .map_err(|e| format!("enable_alarm error: {}", e))?;
    println!("✅ Alarm enabled via TempClient: {}", alarm_enabled);

    // Get sensor info
    let sensor_info = temp_client
        .get_one_sensor(1)
        .await
        .map_err(|e| format!("get_one_sensor error: {}", e))?;
    println!(
        "✅ Sensor info retrieved via TempClient: addr={}, interval={}",
        sensor_info.address, sensor_info.read_interval
    );
    let mut temp_async_client = TempAsyncClient::new(&mut shared_client_manager);

    // Fire an alarm notification
    temp_async_client
        .alarm_fired(1, 75.0)
        .await
        .map_err(|e| format!("alarm_fired error: {}", e))?;
    println!("✅ Alarm fired via TempAsyncClient");

    // Send sensor reading results
    let results = vec![
        SensorReadResult {
            address: 1,
            temp: 25.5,
        },
        SensorReadResult {
            address: 2,
            temp: 30.2,
        },
        SensorReadResult {
            address: 3,
            temp: 28.7,
        },
    ];

    temp_async_client
        .read_results(results)
        .await
        .map_err(|e| format!("read_results error: {}", e))?;
    println!("✅ Multiple sensor results sent via TempAsyncClient");

    // Fire another alarm notification
    temp_async_client
        .alarm_fired(2, 45.5)
        .await
        .map_err(|e| format!("alarm_fired(2) error: {}", e))?;
    println!("✅ Second alarm fired via TempAsyncClient");

    // Send more sensor reading results
    let more_results = vec![
        SensorReadResult {
            address: 4,
            temp: 32.1,
        },
        SensorReadResult {
            address: 5,
            temp: 27.8,
        },
    ];

    temp_async_client
        .read_results(more_results)
        .await
        .map_err(|e| format!("read_results(2) error: {}", e))?;
    println!("✅ Additional sensor results sent via TempAsyncClient");

    // Fire one more alarm with different parameters
    temp_async_client
        .alarm_fired(3, 85.2)
        .await
        .map_err(|e| format!("alarm_fired(3) error: {}", e))?;
    println!("✅ Third alarm fired via TempAsyncClient");

    Ok(())
}
