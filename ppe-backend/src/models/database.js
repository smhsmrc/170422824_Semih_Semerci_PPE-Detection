const { Sequelize, DataTypes } = require('sequelize');
const path = require('path');

// SQLite veritabanı bağlantısı
const sequelize = new Sequelize({
  dialect: 'sqlite',
  storage: path.join(__dirname, '../../database/ppe_detection.db'),
  logging: false // SQL loglarını kapatmak için
});

// User Model
const User = sequelize.define('User', {
  id: {
    type: DataTypes.INTEGER,
    primaryKey: true,
    autoIncrement: true
  },
  username: {
    type: DataTypes.STRING,
    allowNull: false,
    unique: true
  },
  email: {
    type: DataTypes.STRING,
    allowNull: false,
    unique: true,
    validate: {
      isEmail: true
    }
  },
  password: {
    type: DataTypes.STRING,
    allowNull: false
  },
  role: {
    type: DataTypes.ENUM('admin', 'user'),
    defaultValue: 'user'
  },
  isActive: {
    type: DataTypes.BOOLEAN,
    defaultValue: true
  }
});

// Detection Model
const Detection = sequelize.define('Detection', {
  id: {
    type: DataTypes.INTEGER,
    primaryKey: true,
    autoIncrement: true
  },
  filename: {
    type: DataTypes.STRING,
    allowNull: false
  },
  originalName: {
    type: DataTypes.STRING,
    allowNull: false
  },
  fileType: {
    type: DataTypes.ENUM('image', 'video'),
    allowNull: false
  },
  filePath: {
    type: DataTypes.STRING,
    allowNull: false
  },
  fileSize: {
    type: DataTypes.INTEGER
  },
  status: {
    type: DataTypes.ENUM('pending', 'processing', 'completed', 'failed'),
    defaultValue: 'pending'
  },
  processingTime: {
    type: DataTypes.FLOAT // saniye cinsinden
  },
  userId: {
    type: DataTypes.INTEGER,
    references: {
      model: User,
      key: 'id'
    }
  }
});

// DetectionResult Model
const DetectionResult = sequelize.define('DetectionResult', {
  id: {
    type: DataTypes.INTEGER,
    primaryKey: true,
    autoIncrement: true
  },
  detectionId: {
    type: DataTypes.INTEGER,
    references: {
      model: Detection,
      key: 'id'
    }
  },
  objectType: {
    type: DataTypes.STRING, // 'helmet', 'vest', 'person' vb.
    allowNull: false
  },
  confidence: {
    type: DataTypes.FLOAT, // 0-1 arası güven skoru
    allowNull: false
  },
  boundingBox: {
    type: DataTypes.JSON, // {x, y, width, height}
    allowNull: false
  },
  hasViolation: {
    type: DataTypes.BOOLEAN,
    defaultValue: false
  },
  violationType: {
    type: DataTypes.STRING // 'no_helmet', 'no_vest' vb.
  }
});

// Worker Model
const Worker = sequelize.define('Worker', {
  id: {
    type: DataTypes.INTEGER,
    primaryKey: true,
    autoIncrement: true
  },
  workerId: {
    type: DataTypes.STRING,
    allowNull: false,
    unique: true
  },
  name: {
    type: DataTypes.STRING,
    allowNull: false
  },
  email: {
    type: DataTypes.STRING,
    validate: {
      isEmail: true
    }
  },
  phone: {
    type: DataTypes.STRING
  },
  department: {
    type: DataTypes.STRING,
    allowNull: false
  },
  position: {
    type: DataTypes.STRING,
    allowNull: false
  },
  location: {
    type: DataTypes.STRING
  },
  manager: {
    type: DataTypes.STRING
  },
  startDate: {
    type: DataTypes.DATE,
    defaultValue: DataTypes.NOW
  },
  status: {
    type: DataTypes.ENUM('active', 'inactive', 'training'),
    defaultValue: 'active'
  },
  photo: {
    type: DataTypes.TEXT // Base64 encoded image
  },
  assignedPPE: {
    type: DataTypes.JSON, // Array of PPE types
    defaultValue: []
  },
  complianceRate: {
    type: DataTypes.FLOAT,
    defaultValue: 0
  },
  lastSeen: {
    type: DataTypes.DATE
  },
  monthlyViolations: {
    type: DataTypes.INTEGER,
    defaultValue: 0
  },
  totalViolations: {
    type: DataTypes.INTEGER,
    defaultValue: 0
  },
  trainingCompleted: {
    type: DataTypes.BOOLEAN,
    defaultValue: false
  },
  notes: {
    type: DataTypes.TEXT
  }
});

// İlişkiler
User.hasMany(Detection, { foreignKey: 'userId' });
Detection.belongsTo(User, { foreignKey: 'userId' });

Detection.hasMany(DetectionResult, { foreignKey: 'detectionId' });
DetectionResult.belongsTo(Detection, { foreignKey: 'detectionId' });

module.exports = {
  sequelize,
  User,
  Detection,
  DetectionResult,
  Worker
};
