import { IrrigationAPI } from './api';

describe('IrrigationAPI', () => {
  it('should be defined', () => {
    expect(IrrigationAPI).toBeDefined();
  });

  describe('makeRequest endpoint normalization', () => {
    let api: IrrigationAPI;

    beforeEach(() => {
      api = new IrrigationAPI('http://localhost');
      global.fetch = jest.fn().mockResolvedValue({
        ok: true,
        json: () => Promise.resolve({}),
      });
    });

    it('should correctly normalize an endpoint with api/v1 prefix', async () => {
      await api['makeRequest']('api/v1/status');
      expect(global.fetch).toHaveBeenCalledWith('http://localhost/api/v1/status', expect.any(Object));
    });

    it('should correctly normalize an endpoint with api/ prefix', async () => {
      await api['makeRequest']('api/status');
      expect(global.fetch).toHaveBeenCalledWith('http://localhost/api/v1/status', expect.any(Object));
    });

    it('should correctly normalize an endpoint without prefix', async () => {
      await api['makeRequest']('status');
      expect(global.fetch).toHaveBeenCalledWith('http://localhost/api/v1/status', expect.any(Object));
    });

    it('should correctly normalize an endpoint with leading slash', async () => {
      await api['makeRequest']('/status');
      expect(global.fetch).toHaveBeenCalledWith('http://localhost/api/v1/status', expect.any(Object));
    });
  });
});
