use crate::temp_alarm::temp_async_server::{TempAsync, TempAsyncServer};
use crate::temp_alarm::temp_server::{Temp, TempServer};
use crate::temp_alarm::*;
use async_trait::async_trait;
use erpc_rust::{
    codec::BasicCodecFactory,
    server::{MultiTransportServerBuilder, Server},
};

/// Implementation of the Temp service
#[derive(Clone)]
struct TempServiceImpl {
    // In a real implementation, you'd store sensor data here
}

impl TempServiceImpl {
    fn new() -> Self {
        Self {}
    }
}

#[async_trait]
impl Temp for TempServiceImpl {
    async fn add_sensor(
        &self,
        address: u8,
    ) -> Result<bool, Box<dyn std::error::Error + Send + Sync>> {
        println!("🔧 Adding sensor with address: {}", address);
        Ok(true)
    }

    async fn remove_sensor(
        &self,
        address: u8,
    ) -> Result<bool, Box<dyn std::error::Error + Send + Sync>> {
        println!("🗑️  Removing sensor with address: {}", address);
        Ok(true)
    }

    async fn set_interval(
        &self,
        address: u8,
        interval: f32,
    ) -> Result<bool, Box<dyn std::error::Error + Send + Sync>> {
        println!(
            "⏰ Setting interval for sensor {}: {} seconds",
            address, interval
        );
        Ok(true)
    }

    async fn set_alarm(
        &self,
        address: u8,
        alarm_type: AlarmType,
        alarm_temp: f32,
    ) -> Result<bool, Box<dyn std::error::Error + Send + Sync>> {
        println!(
            "🚨 Setting alarm for sensor {}: type {:?}, temp {}",
            address, alarm_type, alarm_temp
        );
        Ok(true)
    }

    async fn enable_alarm(
        &self,
        address: u8,
        alarm_type: AlarmType,
    ) -> Result<bool, Box<dyn std::error::Error + Send + Sync>> {
        println!(
            "✅ Enabling alarm for sensor {}: type {:?}",
            address, alarm_type
        );
        Ok(true)
    }

    async fn disable_alarm(
        &self,
        address: u8,
        alarm_type: AlarmType,
    ) -> Result<bool, Box<dyn std::error::Error + Send + Sync>> {
        println!(
            "❌ Disabling alarm for sensor {}: type {:?}",
            address, alarm_type
        );
        Ok(true)
    }

    async fn get_one_sensor(
        &self,
        address: u8,
    ) -> Result<SensorInfo, Box<dyn std::error::Error + Send + Sync>> {
        println!("📊 Getting info for sensor: {}", address);
        Ok(SensorInfo {
            address,
            read_interval: 1.0,
            high_alarm: AlarmInfo {
                temp: 25.0,
                enabled: false,
            },
            low_alarm: AlarmInfo {
                temp: 10.0,
                enabled: false,
            },
        })
    }

    async fn get_all_sensors_b(
        &self,
    ) -> Result<Vec<SensorInfo>, Box<dyn std::error::Error + Send + Sync>> {
        println!("📋 Getting all sensors");
        Ok(vec![SensorInfo {
            address: 1,
            read_interval: 1.0,
            high_alarm: AlarmInfo {
                temp: 25.0,
                enabled: false,
            },
            low_alarm: AlarmInfo {
                temp: 10.0,
                enabled: false,
            },
        }])
    }

    async fn save_settings(&self) -> Result<Vec<u8>, Box<dyn std::error::Error + Send + Sync>> {
        println!("💾 Saving settings");
        Ok(vec![0x01, 0x02, 0x03]) // Dummy saved state
    }

    async fn load_settings(
        &self,
        saved_state: Vec<u8>,
    ) -> Result<bool, Box<dyn std::error::Error + Send + Sync>> {
        println!("📥 Loading settings: {} bytes", saved_state.len());
        Ok(true)
    }

    async fn read_one_sensor(
        &self,
        address: u8,
    ) -> Result<f32, Box<dyn std::error::Error + Send + Sync>> {
        println!("🌡️  Reading sensor: {}", address);
        // Return a dummy temperature reading
        Ok(22.5)
    }

    async fn read_sensors(
        &self,
        addresses: Vec<u8>,
    ) -> Result<bool, Box<dyn std::error::Error + Send + Sync>> {
        println!("📊 Reading {} sensors:", addresses.len());
        for address in &addresses {
            println!("  🌡️  Sensor address: {}", address);
        }
        Ok(true)
    }
}

/// Implementation of the TempAsync service
#[derive(Clone)]
struct TempAsyncServiceImpl {
    // In a real implementation, you'd store callback state here
}

impl TempAsyncServiceImpl {
    fn new() -> Self {
        Self {}
    }
}

#[async_trait]
impl TempAsync for TempAsyncServiceImpl {
    async fn alarm_fired(
        &self,
        addr: u8,
        temp: f32,
    ) -> Result<(), Box<dyn std::error::Error + Send + Sync>> {
        println!("🚨🔥 ALARM FIRED! Sensor {}: temperature {}", addr, temp);
        Ok(())
    }

    async fn read_results(
        &self,
        results: Vec<SensorReadResult>,
    ) -> Result<(), Box<dyn std::error::Error + Send + Sync>> {
        println!("📊 Received {} sensor readings:", results.len());
        for result in results {
            println!("  🌡️  Sensor {}: {}°C", result.address, result.temp);
        }
        Ok(())
    }
}

/// Main server runner - Multi-Transport Server with TCP support
pub async fn run_server() -> Result<(), Box<dyn std::error::Error + Send + Sync>> {
    use std::sync::Arc;

    let temp_service_impl = TempServiceImpl::new();
    let temp_async_service_impl = TempAsyncServiceImpl::new();
    // Create server with TCP transport
    let mut server = MultiTransportServerBuilder::new()
        .codec_factory(BasicCodecFactory::new())
        // TCP listeners
        .tcp_listener("127.0.0.1:40000") // Main TCP API
        // .serial_port("/dev/ttyUSB0", 115200)  // Primary serial interface (commented out)
        .service(Arc::new(TempServer::new(temp_service_impl)))
        .service(Arc::new(TempAsyncServer::new(temp_async_service_impl)))
        .build()
        .await
        .map_err(|e| format!("Failed to build multi-transport server: {}", e))?;

    println!("🚀 eRPC Server is running on TCP port 40000");
    // Run the server - handles ALL transport types in one loop!
    server.run().await?;
    Ok(())
}
