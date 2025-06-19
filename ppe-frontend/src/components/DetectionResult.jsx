import React from "react";

const DetectionResult = ({ data }) => {
  if (!data || !data.detection) return <p>Veri alınamadı.</p>;

  const { track_id, confidence, status } = data.detection;

  return (
    <div style={{ border: "1px solid #ccc", padding: 20, borderRadius: 8 }}>
      <h2>PPE Tespiti</h2>
      <p><strong>Track ID:</strong> {track_id}</p>
      <p><strong>Confidence:</strong> {confidence}</p>
      <p><strong>Durum:</strong> {status}</p>
    </div>
  );
};

export default DetectionResult;
